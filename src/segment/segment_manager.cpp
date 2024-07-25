#include "segment/segment_manager.h"
#include "common/config.h"
#include "segment/segment.cpp"

namespace logstore {

SegmentManager::SegmentManager(uint32_t segment_num, uint32_t segment_capacity, uint32_t group_num)
    : segment_num_(segment_num),
      segment_capacity_(segment_capacity),
      sealed_segments_(std::make_unique<SealedSegmentList>(group_num)) {
  segments_ = new Segment[segment_num_];
  pba_t s_pba = 0;
  for (uint32_t i = 0; i < segment_num_; i++) {
    segments_[i].Init(i, s_pba, segment_capacity_);
    s_pba += segment_capacity_;
  }
  // open one segment for each group
  for (uint32_t i = 0; i < group_num; i++) {
    opened_segments_.push_back(&segments_[i]);
    free_segments_.remove(&segments_[i]);
  }
}

SegmentManager::~SegmentManager() { delete[] segments_; }

Segment *SegmentManager::GetSegment(uint32_t segment_id) {
  if (segment_id >= segment_num_) {
    return nullptr;
  }
  return &segments_[segment_id];
}

uint32_t SegmentManager::PBA2SegmentId(pba_t pba) { return pba / segment_capacity_; }

off64_t SegmentManager::PBA2SegmentOffset(pba_t pba) { return pba % segment_capacity_; }

Segment *SegmentManager::FindSegment(pba_t pba) {
  uint32_t segment_id = PBA2SegmentId(pba);
  return GetSegment(segment_id);
}

void SegmentManager::MarkPBAInvalid(pba_t pba) {
  Segment *segment = FindSegment(pba);
  if (segment == nullptr) {
    return;
  }
  off64_t offset = PBA2SegmentOffset(pba);
  segment->MarkBlockInvalid(offset);
}

void SegmentManager::MarkPBAValid(pba_t pba, lba_t lba) {
  Segment *segment = FindSegment(pba);
  if (segment == nullptr) {
    return;
  }
  off64_t offset = PBA2SegmentOffset(pba);
  segment->MarkBlockValid(offset, lba);
}

pba_t SegmentManager::GetFreeBlock(uint32_t group_id) {
  LOGSTORE_ASSERT(group_id < opened_segments_.size(), "Invalid group id");
  Segment *segment = opened_segments_[group_id];
  if (segment->IsFull()) {
    sealed_segments_->AddSegment(segment, group_id);
    opened_segments_[group_id] = GetFreeSegment();
    segment = opened_segments_[group_id];
  }
  pba_t pba = segment->GetFreeBlock();
  return pba;
}

Segment *SegmentManager::GetFreeSegment() {
  // Free segment list must not be empty
  LOGSTORE_ASSERT(free_segments_.size() > 0, "No free segment");
  Segment *segment = free_segments_.front();
  free_segments_.pop_front();
  return segment;
}

}  // namespace logstore