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

SegmentManager::SegmentManager(int32_t seg_num, int32_t seg_cap) : seg_num_(seg_num), seg_cap_(seg_cap) {
  l2p_map_ = IndexMapFactory::GetIndexMap(Config::GetInstance().index_map);
  selection_ = SelectionFactory::GetSelection(Config::GetInstance().selection);
  adapter_ = AdapterFactory::GetAdapter(Config::GetInstance().adapter);
  placement_ = PlacementFactory::GetPlacement(Config::GetInstance().placement);

  // Allocate segments
  for (auto i = 0; i < seg_num; i++) {
    std::shared_ptr<Segment> ptr = std::make_shared<Segment>(i, i * seg_cap, seg_cap);  // id, spba, cap
    segments_[i] = ptr;
    free_segments_.insert(ptr);
  }

  // Allocate opened segments from the free segments
  auto opened_num = Config::GetInstance().opened_segment_num;
  for (auto i = 0; i < opened_num; i++) {
    std::shared_ptr<Segment> ptr = *(free_segments_.begin());
    free_segments_.erase(ptr);
    opened_segments_.push_back(ptr);
  }
}

SegmentManager::~SegmentManager() {
  double waf = 1.0 + total_gc_writes_ * 0.1 / total_user_writes_;
  std::cout << "Total User Write: " << total_user_writes_ << std::endl;
  std::cout << "Total GC Write: " << total_gc_writes_ << std::endl;
  std::cout << "WAF: " << waf << std::endl;
}

uint64_t SegmentManager::UserReadBlock(lba_t lba) {
  pba_t pba = SearchL2P(lba);
  if (pba == INVALID_PBA) return 0;
  return adapter_->ReadBlock(nullptr, pba);
}

uint64_t SegmentManager::UserAppendBlock(lba_t lba) {
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
    auto free_seg_ptr = AllocFreeSegment();
    assert(free_seg_ptr != nullptr);
    opened_segments_[group_id] = free_seg_ptr;

    // Seal the full segment
    total_invalid_blocks_ += seg_ptr->GetInvalidBlockCount();
    total_blocks_ += seg_ptr->GetCapacity();
    seg_ptr->SetSealed(true);
    sealed_segments_.insert(seg_ptr);
  }
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
    auto free_seg_ptr = AllocFreeSegment();
    assert(free_seg_ptr != nullptr);
    opened_segments_[group_id] = free_seg_ptr;

    // Seal the full segment
    total_invalid_blocks_ += seg_ptr->GetInvalidBlockCount();
    total_blocks_ += seg_ptr->GetCapacity();
    seg_ptr->SetSealed(true);
    sealed_segments_.insert(seg_ptr);
  }
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

std::shared_ptr<Segment> SegmentManager::AllocFreeSegment() {
  if (free_segments_.empty()) {
    std::cerr << "No free segment" << std::endl;
    return nullptr;
  }
  auto seg_ptr = *(free_segments_.begin());
  free_segments_.erase(seg_ptr);
  return seg_ptr;
}

pba_t SegmentManager::SearchL2P(lba_t lba) const { return l2p_map_->Query(lba); }

void SegmentManager::UpdateL2P(lba_t lba, pba_t pba) { l2p_map_->Update(lba, pba); }

// void SegmentManager::DoGCLeftWork(seg_id_t victim_id) {
//   sealed_segments_.remove(victim_id);
//   free_segments_.push_back(victim_id);
//   Segment *victim = GetSegment(victim_id);
//   total_invalid_block_num_ -= victim->GetInvalidBlockCount();
//   victim->ClearMetadata();
// }

// void SegmentManager::PrintSegmentsInfo() {
//   std::cout << "Opened segments num: " << opened_segments_.size() << std::endl;
//   for (auto it = opened_segments_.begin(); it != opened_segments_.end(); it++) {
//     GetSegment(*it)->PrintSegmentInfo();
//   }
//   std::cout << std::endl;
//   std::cout << "Sealed segments num: " << sealed_segments_.size() << std::endl;
//   for (auto it = sealed_segments_.begin(); it != sealed_segments_.end(); it++) {
//     GetSegment(*it)->PrintSegmentInfo();
//   }
//   std::cout << std::endl;
//   std::cout << "Free segments num: " << free_segments_.size() << std::endl;
//   for (auto it = free_segments_.begin(); it != free_segments_.end(); it++) {
//     GetSegment(*it)->PrintSegmentInfo();
//   }
// }

}  // namespace logstore