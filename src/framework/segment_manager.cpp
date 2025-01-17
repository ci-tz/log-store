#include <iostream>

#include "common/config.h"
#include "common/logger.h"
#include "common/macros.h"
#include "framework/phy_segment.h"
#include "framework/segment.h"
#include "framework/segment_manager.h"
#include "index/indexmap_factory.h"
#include "placement/placement_factory.h"
#include "select/selection_factory.h"
#include "storage/adapter/adapter_factory.h"

// #define PROBE 1

namespace logstore {

uint64_t SegmentManager::write_timestamp = 0;

SegmentManager::SegmentManager() {
  const auto &config = Config::GetInstance();
  seg_num_ = config.seg_num;
  seg_cap_ = config.seg_cap;
  op_ratio_ = config.op;
  l2p_map_ = IndexMapFactory::GetIndexMap(Config::GetInstance().index_map);
  selection_ = SelectionFactory::GetSelection(Config::GetInstance().selection);
  adapter_ = AdapterFactory::GetAdapter(Config::GetInstance().adapter);
  placement_ = PlacementFactory::GetPlacement(Config::GetInstance().placement);
#ifdef PROBE
  probe_ = std::make_shared<GcLifespan>();  // Probe
#endif

  // Allocate segments
  for (auto i = 0; i < seg_num_; i++) {
    auto ptr = std::make_shared<PhySegment>(i, i * seg_cap_, seg_cap_);
    phy_segments_[i] = ptr;
    free_phy_segments_.insert(ptr);
  }

  InitOpenedSegments();
}

SegmentManager::~SegmentManager() {
  double waf = 1.0 + total_gc_writes_ * 1.0 / total_user_writes_;
  LOG_INFO("Total User Write: %ld", total_user_writes_);
  LOG_INFO("Total GC Write: %ld", total_gc_writes_);
  LOG_INFO("WAF: %.3f", waf);
#ifdef PROBE
  probe_->PrintCount();            // Probe
  probe_->PrintAverageLifespan();  // Probe
#endif
}

void SegmentManager::InitOpenedSegments() {
  int32_t max_class = Config::GetInstance().max_class;
  int32_t max_level = (max_class + 1) * 2;
  for (int32_t level = 0, class_id = max_class; level < max_level; level++) {
    // Get a free physical segment
    auto ptr = *(free_phy_segments_.begin());
    free_phy_segments_.erase(ptr);

    ptr->OpenAs(class_id);
    auto &sub_segs = ptr->GetSubSegments();
    int32_t sub_seg_num = sub_segs.size();
    LOGSTORE_ASSERT(sub_seg_num == 1 << class_id, "Sub segment number mismatch");

    for (int i = 0; i < sub_seg_num; i++) {
      auto sub_seg_ptr = sub_segs[i];
      opened_segments_[level].push_back(sub_seg_ptr);
    }

    write_pointers_[level] = 0;

    if (level % 2) {
      class_id--;
    }
  }
}

std::shared_ptr<Segment> SegmentManager::GetSegment(pba_t pba) {
  seg_id_t sid = pba / seg_cap_;
  auto it = phy_segments_.find(sid);
  if (it == phy_segments_.end()) {
    return nullptr;
  }
  auto phy_seg_ptr = it->second;
  return phy_seg_ptr->GetSegment(pba);
}

uint64_t SegmentManager::UserReadBlock(lba_t lba) {
  pba_t pba = SearchL2P(lba);
  if (pba == INVALID_PBA) return 0;
  return adapter_->ReadBlock(nullptr, pba);
}

pba_t SegmentManager::LevelAppendBlock(lba_t lba, level_id_t level) {
  auto &seg_vector = opened_segments_[level];
  int32_t opened_num = seg_vector.size();

  int32_t wp = write_pointers_[level];
  write_pointers_[level] = (wp + 1) % opened_num;

  auto seg_ptr = seg_vector[wp];
  LOGSTORE_ASSERT(seg_ptr != nullptr && !seg_ptr->IsFull(), "Segment is full");
  lba_t lba = seg_ptr->AppendBlock(lba);

  if (seg_ptr->IsFull()) {
    // Alloc a new segment
    int32_t class_id = seg_ptr->GetClassID();
    auto new_seg_ptr = AllocFreeSegment(class_id, wp);
    LOGSTORE_ASSERT(new_seg_ptr != nullptr, "Segment not found");
    seg_vector[wp] = new_seg_ptr;

    // Seal the full segment
    total_invalid_blocks_ += seg_ptr->GetIBC();
    total_blocks_ += seg_ptr->GetCapacity();
    seg_ptr->SetSealed(true);
    sealed_segments_[class_id].AddSegment(seg_ptr, wp);
  }

  return lba;
}

uint64_t SegmentManager::UserAppendBlock(lba_t lba) {
  while (ShouldGc() == 2) {
    DoGc(true);
  }

  LOG_DEBUG("Write LBA: %d", lba);

  pba_t old_pba = SearchL2P(lba);
  if (old_pba != INVALID_PBA) {
    auto seg_ptr = GetSegment(old_pba);
    LOGSTORE_ASSERT(seg_ptr != nullptr, "Segment not found");
    seg_ptr->MarkBlockInvalid(old_pba);
    if (seg_ptr->IsSealed()) {
      total_invalid_blocks_++;
    }
  }

  level_id_t level = placement_->Classify(lba, false);
  pba_t new_pba = LevelAppendBlock(lba, level);
  UpdateL2P(lba, new_pba);
  placement_->MarkUserAppend(lba, write_timestamp++);
#ifdef PROBE
  probe_->MarkUserWrite(lba, write_timestamp - 1);  // Probe
#endif
  total_user_writes_++;

  while (ShouldGc() == 1 && DoGc(false)) {
    ;
  }
  return adapter_->WriteBlock(nullptr, new_pba);  // TODO: 添加延迟模拟
}

void SegmentManager::GcAppendBlock(lba_t lba, pba_t old_pba) {
  LOGSTORE_ASSERT(SearchL2P(lba) == old_pba, "Old PBA not match L2P");
  placement_->MarkGcAppend(lba);
#ifdef PROBE
  probe_->MarkGc(lba);  // Probe
#endif

  int32_t group_id = placement_->Classify(lba, true);
  auto seg_ptr = opened_segments_[group_id];
  pba_t new_pba = seg_ptr->AppendBlock(lba);
  UpdateL2P(lba, new_pba);
  total_gc_writes_++;

  if (seg_ptr->IsFull()) {
    // Open a new segment
    auto free_seg_ptr = AllocFreeSegment(group_id);
    LOGSTORE_ASSERT(free_seg_ptr != nullptr, "No free segment");
    opened_segments_[group_id] = free_seg_ptr;
    LOGSTORE_ASSERT(free_seg_ptr->GetGroupID() == group_id, "Segment group id not match");

    // Seal the full segment
    total_invalid_blocks_ += seg_ptr->GetInvalidBlockCount();
    total_blocks_ += seg_ptr->GetCapacity();
    seg_ptr->SetSealed(true);
    sealed_segments_.insert(seg_ptr);
  }
  adapter_->WriteBlock(nullptr, new_pba);  // TODO: 添加延迟模拟
}

seg_id_t SegmentManager::SelectVictimSegment() { return selection_->Select(sealed_segments_); }

std::shared_ptr<Segment> SegmentManager::AllocFreeSegment(class_id_t class_id, int32_t wp) {
  SegmentLists &seg_lists = free_segments_[class_id];
  if (seg_lists.IsEmpty(wp)) {
    AllocFreePhySegment(class_id);
  }
  return seg_lists.GetSegment(wp);
}

void SegmentManager::AllocFreePhySegment(class_id_t class_id) {
  LOGSTORE_ASSERT(!free_phy_segments_.empty(), "No free physical segment");
  auto phy_seg_ptr = *(free_phy_segments_.begin());
  free_phy_segments_.erase(phy_seg_ptr);

  auto &seg_vector = phy_seg_ptr->OpenAs(class_id);
  int32_t num = seg_vector.size();

  auto &free_seg_lists = free_segments_[class_id];
  for (int wp = 0; wp < num; wp++) {
    free_seg_lists.AddSegment(seg_vector[wp], wp);
  }
}

pba_t SegmentManager::SearchL2P(lba_t lba) const { return l2p_map_->Query(lba); }

void SegmentManager::UpdateL2P(lba_t lba, pba_t pba) { l2p_map_->Update(lba, pba); }

int32_t SegmentManager::ShouldGc() {
  int32_t free_seg_num = free_phy_segments_.size();
  int32_t threshold_bg = static_cast<int32_t>(seg_num_ * op_ratio_ * 0.75);
  int32_t threshold_force = static_cast<int32_t>(seg_num_ * op_ratio_ * 0.10);
  threshold_bg = std::max(threshold_bg, 1);
  threshold_force = std::max(threshold_force, 1);

  if (free_seg_num > threshold_bg) {  // enough free segments
    return 0;
  } else if (free_seg_num > threshold_force) {  // bg gc
    return 1;
  } else {  // force gc
    return 2;
  }
}

void SegmentManager::GcReadSegment(seg_id_t victim) {
  auto victim_ptr = segments_[victim];
  LOGSTORE_ASSERT(victim_ptr != nullptr && victim_ptr->IsSealed(), "Victim segment not sealed");
  auto capacity = victim_ptr->GetCapacity();
  for (int32_t offset = 0; offset < capacity; offset++) {
    lba_t lba = victim_ptr->GetLBA(offset);
    if (lba == INVALID_LBA) {
      continue;
    }
    pba_t pba = victim_ptr->GetPBA(offset);
    LOGSTORE_ASSERT(pba == SearchL2P(lba), "L2P not match");
    adapter_->ReadBlock(nullptr, pba);
  }
}

bool SegmentManager::DoGc(bool force) {
  seg_id_t victim = SelectVictimSegment();
  LOGSTORE_ENSURE(victim != -1, "No valid segment to gc");
  auto victim_ptr = segments_[victim];
  LOGSTORE_ENSURE(victim_ptr != nullptr && victim_ptr->IsSealed(), "Victim segment not sealed");

  double gc_ratio = victim_ptr->GetInvalidBlockCount() * 1.0 / victim_ptr->GetCapacity();
  if (!force && gc_ratio < 0.15) {
    return false;
  }
  if (force) {
    LOG_DEBUG("FG GC: Victim segment: %d, Gp: %.2f", victim, gc_ratio);
  } else {
    LOG_DEBUG("BG GC: Victim segment: %d, Gp: %.2f", victim, gc_ratio);
  }

  placement_->MarkCollectSegment(victim_ptr);
  GcReadSegment(victim);

  int32_t capacity = victim_ptr->GetCapacity();
  for (int32_t offset = 0; offset < capacity; offset++) {
    lba_t lba = victim_ptr->GetLBA(offset);
    if (lba == INVALID_LBA) {
      continue;
    }
    pba_t old_pba = victim_ptr->GetPBA(offset);
    LOGSTORE_ASSERT(old_pba == SearchL2P(lba), "L2P not match");
    GcAppendBlock(lba, old_pba);
  }

  total_blocks_ -= capacity;
  total_invalid_blocks_ -= victim_ptr->GetInvalidBlockCount();

  GcEraseSegment(victim);

  sealed_segments_.erase(victim_ptr);
  free_segments_.insert(victim_ptr);
  return true;
}

void SegmentManager::GcEraseSegment(seg_id_t victim) {
  auto victim_ptr = segments_[victim];
  LOGSTORE_ASSERT(victim_ptr != nullptr && victim_ptr->IsSealed(), "Victim segment not sealed");
  victim_ptr->EraseSegment();

  adapter_->EraseSegment(victim);
}

void SegmentManager::PrintSegmentsInfo() {
  std::lock_guard<std::mutex> lock(global_mutex_);

  std::cout << "--------------------------------------" << std::endl;
  std::cout << "----------Opened segments num: " << opened_segments_.size() << "----------" << std::endl;
  for (auto it = opened_segments_.begin(); it != opened_segments_.end(); it++) {
    (*it)->PrintSegmentInfo();
  }
  std::cout << "----------Sealed segments num: " << sealed_segments_.size() << "----------" << std::endl;
  for (auto it = sealed_segments_.begin(); it != sealed_segments_.end(); it++) {
    (*it)->PrintSegmentInfo();
  }
  std::cout << "----------Free segments num: " << free_segments_.size() << "----------" << std::endl;
  for (auto it = free_segments_.begin(); it != free_segments_.end(); it++) {
    (*it)->PrintSegmentInfo();
  }
  std::cout << "--------------------------------------" << std::endl;
}

}  // namespace logstore