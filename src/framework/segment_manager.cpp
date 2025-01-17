#include <iostream>

#include "common/config.h"
#include "common/logger.h"
#include "common/macros.h"
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
    std::shared_ptr<Segment> ptr = std::make_shared<Segment>(i, i * seg_cap_, seg_cap_);  // id, spba, cap
    segments_[i] = ptr;
    free_segments_.insert(ptr);
  }

  // Allocate opened segments from the free segments
  auto opened_num = Config::GetInstance().opened_segment_num;
  for (auto i = 0; i < opened_num; i++) {
    std::shared_ptr<Segment> ptr = *(free_segments_.begin());
    free_segments_.erase(ptr);
    ptr->InitSegment(write_timestamp, i);
    opened_segments_.push_back(ptr);
  }
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

uint64_t SegmentManager::UserReadBlock(lba_t lba) {
  pba_t pba = SearchL2P(lba);
  if (pba == INVALID_PBA) return 0;
  return adapter_->ReadBlock(nullptr, pba);
}

uint64_t SegmentManager::UserAppendBlock(lba_t lba) {
  // 若当前需要进行FG GC，则线程阻塞
  // cv_.wait(lock, [this]() { return ShouldGc() != 2; });
  while (ShouldGc() == 2) {
    DoGc(true);
  }

  LOG_DEBUG("Write LBA: %d", lba);

  pba_t old_pba = SearchL2P(lba);
  if (old_pba != INVALID_PBA) {
    auto seg_ptr = GetSegment(old_pba);
    LOGSTORE_ASSERT(seg_ptr != nullptr, "Segment not found");
    off64_t offset = old_pba % seg_cap_;
    seg_ptr->MarkBlockInvalid(offset);
    if (seg_ptr->IsSealed()) {
      total_invalid_blocks_++;
    }
  }

  int32_t group_id = placement_->Classify(lba, false);
  auto seg_ptr = opened_segments_[group_id];
  pba_t new_pba = seg_ptr->AppendBlock(lba);
  UpdateL2P(lba, new_pba);
  placement_->MarkUserAppend(lba, write_timestamp++);
#ifdef PROBE
  probe_->MarkUserWrite(lba, write_timestamp - 1);  // Probe
#endif
  total_user_writes_++;

  if (seg_ptr->IsFull()) {
    // Open a new segment
    auto free_seg_ptr = AllocFreeSegment(group_id);
    LOGSTORE_ASSERT(free_seg_ptr != nullptr, "No free segment");
    opened_segments_[group_id] = free_seg_ptr;

    // Seal the full segment
    total_invalid_blocks_ += seg_ptr->GetInvalidBlockCount();
    total_blocks_ += seg_ptr->GetCapacity();
    seg_ptr->SetSealed(true);
    sealed_segments_.insert(seg_ptr);
  }
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

std::shared_ptr<Segment> SegmentManager::GetSegment(pba_t pba) {
  seg_id_t sid = pba / seg_cap_;
  auto it = segments_.find(sid);
  if (it == segments_.end()) {
    return nullptr;
  } else {
    return it->second;
  }
}

seg_id_t SegmentManager::SelectVictimSegment() { return selection_->Select(sealed_segments_); }

std::shared_ptr<Segment> SegmentManager::AllocFreeSegment(int32_t group_id) {
  if (free_segments_.empty()) {
    std::cerr << "No free segment" << std::endl;
    return nullptr;
  }

  auto seg_ptr = *(free_segments_.begin());
  free_segments_.erase(seg_ptr);
  seg_ptr->InitSegment(write_timestamp, group_id);
  return seg_ptr;
}

pba_t SegmentManager::SearchL2P(lba_t lba) const { return l2p_map_->Query(lba); }

void SegmentManager::UpdateL2P(lba_t lba, pba_t pba) { l2p_map_->Update(lba, pba); }

int32_t SegmentManager::ShouldGc() {
  int32_t free_seg_num = free_segments_.size();
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