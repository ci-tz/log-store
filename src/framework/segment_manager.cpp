#include <iostream>

#include "common/config.h"
#include "framework/segment.h"
#include "framework/segment_manager.h"
#include "index/indexmap_factory.h"
#include "placement/placement_factory.h"
#include "select/selection_factory.h"
#include "storage/adapter/adapter_factory.h"

namespace logstore {

uint64_t SegmentManager::write_timestamp = 0;

SegmentManager::SegmentManager(int32_t seg_num, int32_t seg_cap, double op)
    : seg_num_(seg_num), seg_cap_(seg_cap), op_ratio_(op) {
  l2p_map_ = IndexMapFactory::GetIndexMap(Config::GetInstance().index_map);
  selection_ = SelectionFactory::GetSelection(Config::GetInstance().selection);
  adapter_ = AdapterFactory::GetAdapter(Config::GetInstance().adapter);
  placement_ = PlacementFactory::GetPlacement(Config::GetInstance().placement);

  // Allocate segments
  for (auto i = 0; i < seg_num_; i++) {
    std::shared_ptr<Segment> ptr = std::make_shared<Segment>(i, i * seg_cap, seg_cap);  // id, spba, cap
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
  std::cout << "Total User Write: " << total_user_writes_ << std::endl;
  std::cout << "Total GC Write: " << total_gc_writes_ << std::endl;
  std::cout << "WAF: " << waf << std::endl;
}

uint64_t SegmentManager::UserReadBlock(lba_t lba) {
  std::lock_guard<std::mutex> lock(global_mutex_);
  pba_t pba = SearchL2P(lba);
  if (pba == INVALID_PBA) return 0;
  return adapter_->ReadBlock(nullptr, pba);
}

uint64_t SegmentManager::UserAppendBlock(lba_t lba) {
  std::unique_lock<std::mutex> lock(global_mutex_);

  // 若当前需要进行GC，则线程阻塞
  while (ShouldGc() == 1) {
    cv_.wait(lock);
  }

  pba_t old_pba = SearchL2P(lba);
  if (old_pba != INVALID_PBA) {
    auto seg_ptr = GetSegment(old_pba);
    assert(seg_ptr != nullptr);
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
  total_user_writes_++;

  if (seg_ptr->IsFull()) {
    // Open a new segment
    auto free_seg_ptr = AllocFreeSegment(group_id);
    assert(free_seg_ptr != nullptr);
    opened_segments_[group_id] = free_seg_ptr;

    // Seal the full segment
    total_invalid_blocks_ += seg_ptr->GetInvalidBlockCount();
    total_blocks_ += seg_ptr->GetCapacity();
    seg_ptr->SetSealed(true);
    sealed_segments_.insert(seg_ptr);
  }
  return adapter_->WriteBlock(nullptr, new_pba);  // TODO: 添加延迟模拟
}

void SegmentManager::GcAppendBlock(lba_t lba, pba_t old_pba) {
  assert(SearchL2P(lba) == old_pba);
  placement_->MarkGcAppend(lba);

  int32_t group_id = placement_->Classify(lba, true);
  auto seg_ptr = opened_segments_[group_id];
  pba_t new_pba = seg_ptr->AppendBlock(lba);
  UpdateL2P(lba, new_pba);
  total_gc_writes_++;

  if (seg_ptr->IsFull()) {
    // Open a new segment
    auto free_seg_ptr = AllocFreeSegment(group_id);
    assert(free_seg_ptr != nullptr);
    opened_segments_[group_id] = free_seg_ptr;
    assert(free_seg_ptr->GetGroupID() == group_id);

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
  int32_t threshold = static_cast<int32_t>(seg_num_ * op_ratio_ * 0.25);
  threshold = std::max(threshold, 1);
  if (free_seg_num < threshold) {
    std::cout << "ForceGC: Free segment num: " << free_seg_num << std::endl;
    return 1;
  }

  double gp = total_invalid_blocks_ * 1.0 / total_blocks_;
  if (gp > 0.15) {
    std::cout << "BgGC: IBC:" << total_invalid_blocks_ << ", TB: " << total_blocks_ << std::endl;
    return 2;
  }

  return 0;
}

void SegmentManager::GcReadSegment(seg_id_t victim) {
  auto victim_ptr = segments_[victim];
  assert(victim_ptr != nullptr && victim_ptr->IsSealed());
  auto capacity = victim_ptr->GetCapacity();
  for (int32_t offset = 0; offset < capacity; offset++) {
    lba_t lba = victim_ptr->GetLBA(offset);
    if (lba == INVALID_LBA) {
      continue;
    }
    pba_t pba = victim_ptr->GetPBA(offset);
    assert(pba == SearchL2P(lba));
    adapter_->ReadBlock(nullptr, pba);
  }
}

void SegmentManager::DoGc() {
  seg_id_t victim = SelectVictimSegment();
  assert(victim != -1);
  auto victim_ptr = segments_[victim];
  assert(victim_ptr != nullptr && victim_ptr->IsSealed());
  std::cout << "GC: Victim segment: " << victim << std::endl;

  placement_->MarkCollectSegment(victim_ptr);
  GcReadSegment(victim);

  int32_t capacity = victim_ptr->GetCapacity();
  for (int32_t offset = 0; offset < capacity; offset++) {
    lba_t lba = victim_ptr->GetLBA(offset);
    if (lba == INVALID_LBA) {
      continue;
    }
    pba_t old_pba = victim_ptr->GetPBA(offset);
    assert(old_pba == SearchL2P(lba));
    GcAppendBlock(lba, old_pba);
  }

  total_blocks_ -= capacity;
  total_invalid_blocks_ -= victim_ptr->GetInvalidBlockCount();

  GcEraseSegment(victim);

  sealed_segments_.erase(victim_ptr);
  free_segments_.insert(victim_ptr);
}

void SegmentManager::GcEraseSegment(seg_id_t victim) {
  auto victim_ptr = segments_[victim];
  assert(victim_ptr != nullptr && victim_ptr->IsSealed());
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