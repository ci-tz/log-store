#include "segment/segment_manager.h"
#include <iostream>
#include "common/config.h"
#include "segment/segment.h"
#include "select/select_segment_factory.h"

namespace logstore {

SegmentManager::SegmentManager(int32_t segment_num, int32_t segment_capacity,
                               std::shared_ptr<SelectSegment> select)
    : segment_num_(segment_num), segment_capacity_(segment_capacity), select_(select) {
  segments_ = new Segment[segment_num_];
  pba_t s_pba = 0;
  for (int32_t i = 0; i < segment_num_; i++) {
    segments_[i].Init(i, s_pba, segment_capacity_);
    free_segments_.push_back(i);
    s_pba += segment_capacity_;
  }
  // open one segment
  seg_id_t segment_id = AllocateFreeSegment();
  opened_segments_.push_back(segment_id);
}

void SegmentManager::SetSelectStrategy(std::shared_ptr<SelectSegment> select) { select_ = select; }

SegmentManager::~SegmentManager() { delete[] segments_; }

Segment *SegmentManager::GetSegment(seg_id_t segment_id) {
  if (segment_id >= segment_num_) {
    return nullptr;
  }
  return &segments_[segment_id];
}

seg_id_t SegmentManager::GetSegmentId(pba_t pba) const { return pba / segment_capacity_; }

off64_t SegmentManager::GetOffset(pba_t pba) const { return pba % segment_capacity_; }

Segment *SegmentManager::FindSegment(pba_t pba) { return GetSegment(GetSegmentId(pba)); }

void SegmentManager::MarkBlockInvalid(pba_t pba) {
  seg_id_t sid = GetSegmentId(pba);
  off64_t offset = GetOffset(pba);
  Segment *segment = GetSegment(sid);
  if (segment == nullptr) {
    return;
  }
  segment->MarkBlockInvalid(offset);
}

void SegmentManager::MarkBlockValid(pba_t pba, lba_t lba) {
  seg_id_t sid = GetSegmentId(pba);
  off64_t offset = GetOffset(pba);
  Segment *segment = GetSegment(sid);
  if (segment == nullptr) {
    return;
  }
  segment->MarkBlockValid(offset, lba);
}

bool SegmentManager::IsValid(pba_t pba) {
  Segment *segment = FindSegment(pba);
  if (segment == nullptr) {
    return false;
  }
  off64_t offset = GetOffset(pba);
  return segment->IsValid(offset);
}

pba_t SegmentManager::AllocateFreeBlock() {
  LOGSTORE_ASSERT(opened_segments_.size() > 0, "No opened segment");
  seg_id_t segment_id = opened_segments_.front();
  Segment *segment = GetSegment(segment_id);
  if (segment->IsFull()) {
    opened_segments_.pop_front();
    sealed_segments_.push_back(segment_id);
    seg_id_t free_segment_id = AllocateFreeSegment();
    opened_segments_.push_back(free_segment_id);
    segment = GetSegment(free_segment_id);
  }
  pba_t pba = segment->AllocateFreeBlock();
  LOGSTORE_ASSERT(pba != INVALID_PBA, "Allocate block failed");
  return pba;
}

seg_id_t SegmentManager::AllocateFreeSegment() {
  // Free segment list must not be empty
  LOGSTORE_ASSERT(free_segments_.size() > 0, "No free segment");
  seg_id_t segment_id = free_segments_.front();
  free_segments_.pop_front();
  return segment_id;
}

double SegmentManager::GetFreeSegmentRatio() const {
  return static_cast<double>(free_segments_.size()) / segment_num_;
}

seg_id_t SegmentManager::SelectVictimSegment() {
  return select_->do_select(sealed_segments_, segments_);
}

void SegmentManager::DoGCLeftWork(seg_id_t victim_id) {
  sealed_segments_.remove(victim_id);
  free_segments_.push_back(victim_id);
  Segment *victim = GetSegment(victim_id);
  victim->ClearMetadata();
}

void SegmentManager::PrintSegmentsInfo() {
  std::cout << "Opened segments num: " << opened_segments_.size() << std::endl;
  for (auto it = opened_segments_.begin(); it != opened_segments_.end(); it++) {
    GetSegment(*it)->PrintSegmentInfo();
  }
  std::cout << std::endl;
  std::cout << "Sealed segments num: " << sealed_segments_.size() << std::endl;
  for (auto it = sealed_segments_.begin(); it != sealed_segments_.end(); it++) {
    GetSegment(*it)->PrintSegmentInfo();
  }
  std::cout << std::endl;
  std::cout << "Free segments num: " << free_segments_.size() << std::endl;
  for (auto it = free_segments_.begin(); it != free_segments_.end(); it++) {
    GetSegment(*it)->PrintSegmentInfo();
  }
}

}  // namespace logstore