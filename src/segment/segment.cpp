#include "segment/segment.h"
#include "common/config.h"

namespace logstore {

Segment::Segment(uint32_t segment_id, pba_t s_pba, uint32_t capacity) {
  Init(segment_id, s_pba, capacity);
}

void Segment::Init(uint32_t segment_id, pba_t s_pba, uint32_t capacity) {
  segment_id_ = segment_id;
  s_pba_ = s_pba;
  capacity_ = capacity;
  rmap_ = std::make_unique<lba_t[]>(capacity_);
  for (uint32_t i = 0; i < capacity_; i++) {
    rmap_[i] = INVALID_LBA;
  }
}

// Clear the metadata of the segment
void Segment::Clear() {
  // clear rmap_
  for (uint32_t i = 0; i < capacity_; i++) {
    rmap_[i] = INVALID_LBA;
  }
  create_timestamp_ = 0;
  group_id_ = -1;
  next_append_offset_ = 0;
  sealed_ = false;
  invalid_block_count_ = 0;
}

pba_t Segment::AllocateFreeBlock() {
  if (IsFull()) {
    return INVALID_PBA;
  }
  pba_t pba = s_pba_ + next_append_offset_++;
  if (next_append_offset_ == capacity_) {
    sealed_ = true;
  }
  return pba;
}

void Segment::MarkBlockValid(off64_t offset, lba_t lba) {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  LOGSTORE_ASSERT(rmap_[offset] == INVALID_LBA, "Block is already valid");
  rmap_[offset] = lba;
}

void Segment::MarkBlockInvalid(off64_t offset) {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  LOGSTORE_ASSERT(rmap_[offset] != INVALID_LBA, "Block is already invalid");
  rmap_[offset] = INVALID_LBA;
  invalid_block_count_++;
}

bool Segment::IsValid(off64_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  return rmap_[offset] != INVALID_LBA;
}

bool Segment::IsFull() const { return next_append_offset_ == capacity_; }

double Segment::GetGarbageRatio() const {
  return static_cast<double>(invalid_block_count_) / Size();
}

uint64_t Segment::GetCreateTime() const { return create_timestamp_; }

uint32_t Segment::GetSegmentId() const { return segment_id_; }

uint32_t Segment::GetGroupID() const { return group_id_; }

uint32_t Segment::GetCapacity() const { return capacity_; }

bool Segment::IsSealed() const { return sealed_; }

uint32_t Segment::Size() const { return next_append_offset_; }

pba_t Segment::GetStartPBA() const { return s_pba_; }

lba_t Segment::GetOffsetLBA(off64_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  return rmap_[offset];
}

void Segment::SetGroupID(uint32_t group_id) { group_id_ = group_id; }

void Segment::SetCreateTime(uint64_t create_time) { create_timestamp_ = create_time; }

}  // namespace logstore