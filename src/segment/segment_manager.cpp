#include "segment/segment_manager.h"
#include "common/config.h"
#include "segment/segment.cpp"

namespace logstore {

SegmentManager::SegmentManager(uint32_t segment_num, uint32_t segment_capacity)
    : segment_num_(segment_num), segment_capacity_(segment_capacity) {
  segments_ = new Segment[segment_num_];
  pba_t s_pba = 0;
  for (uint32_t i = 0; i < segment_num_; i++, s_pba += segment_capacity_) {
    segments_[i].Init(i, s_pba, segment_capacity_);
    free_segments_.push_back(&segments_[i]);
  }
  // open one segment
  Segment *segment = free_segments_.front();
  free_segments_.pop_front();
  opened_segments_.push_back(segment);
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

void SegmentManager::MarkBlockInvalid(pba_t pba) {
  Segment *segment = FindSegment(pba);
  if (segment == nullptr) {
    return;
  }
  off64_t offset = PBA2SegmentOffset(pba);
  segment->MarkBlockInvalid(offset);
}

void SegmentManager::MarkBlockValid(pba_t pba, lba_t lba) {
  Segment *segment = FindSegment(pba);
  if (segment == nullptr) {
    return;
  }
  off64_t offset = PBA2SegmentOffset(pba);
  segment->MarkBlockValid(offset, lba);
}

pba_t SegmentManager::AllocateFreeBlock() {
  LOGSTORE_ASSERT(opened_segments_.size() > 0, "No opened segment");
  Segment *segment = opened_segments_.front();
  if (segment->IsFull()) {
    opened_segments_.pop_front();
    sealed_segments_.push_back(segment);
    opened_segments_.push_back(GetFreeSegment());
    segment = opened_segments_.front();
  }
  pba_t pba = segment->AllocateFreeBlock();
  return pba;
}

Segment *SegmentManager::GetFreeSegment() {
  // Free segment list must not be empty
  LOGSTORE_ASSERT(free_segments_.size() > 0, "No free segment");
  Segment *segment = free_segments_.front();
  free_segments_.pop_front();
  return segment;
}

double SegmentManager::GetFreeSegmentRatio() const {
  return static_cast<double>(free_segments_.size()) / segment_num_;
}

Segment *SegmentManager::SelectVictimSegment() {
  // TODO: use a selection algorithm class to do this
  return nullptr;
}

}  // namespace logstore