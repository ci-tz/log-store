#include "framework/segment.h"
#include <iostream>
#include "common/config.h"

namespace logstore {

Segment::Segment(seg_id_t segment_id, pba_t s_pba, uint32_t capacity) {
  Init(segment_id, s_pba, capacity);
}

void Segment::Init(seg_id_t segment_id, pba_t s_pba, uint32_t capacity) {
  segment_id_ = segment_id;
  s_pba_ = s_pba;
  capacity_ = capacity;
  rmap_ = std::make_unique<lba_t[]>(capacity_);
  for (int32_t i = 0; i < capacity_; i++) {
    rmap_[i] = INVALID_LBA;
  }
}

void Segment::ClearMetadata() {
  for (int32_t i = 0; i < capacity_; i++) {
    rmap_[i] = INVALID_LBA;
  }
  create_timestamp_ = 0;
  group_id_ = -1;
  next_append_offset_ = 0;
  invalid_block_count_ = 0;
}

pba_t Segment::AllocateFreeBlock() {
  if (IsFull()) {
    return INVALID_PBA;
  }
  pba_t pba = s_pba_ + next_append_offset_++;
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

bool Segment::IsFull() const { return Size() == capacity_; }

double Segment::GetGarbageRatio() const {
  return static_cast<double>(invalid_block_count_) / Size();
}

uint64_t Segment::GetCreateTime() const { return create_timestamp_; }

seg_id_t Segment::GetSegmentId() const { return segment_id_; }

int32_t Segment::GetGroupID() const { return group_id_; }

int32_t Segment::GetCapacity() const { return capacity_; }

int32_t Segment::Size() const { return next_append_offset_; }

pba_t Segment::GetStartPBA() const { return s_pba_; }

pba_t Segment::GetPBA(off64_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  return s_pba_ + offset;
}

lba_t Segment::GetLBA(off64_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  return rmap_[offset];
}

void Segment::SetGroupID(int32_t group_id) { group_id_ = group_id; }

void Segment::SetCreateTime(uint64_t create_time) { create_timestamp_ = create_time; }

void Segment::PrintSegmentInfo() const {
  std::cout << "-------------------" << std::endl;
  std::cout << "Segment ID: " << segment_id_ << std::endl;
  std::cout << "Create Time: " << create_timestamp_ << std::endl;
  std::cout << "Next Append Offset: " << next_append_offset_ << std::endl;
  std::cout << "Invalid Block Count: " << invalid_block_count_ << std::endl;
  for (int32_t i = 0; i < capacity_; i++) {
    lba_t lba = rmap_[i];
    if (lba == INVALID_LBA) {
      std::cout << "I ";
    } else {
      std::cout << lba << " ";
    }
  }
  std::cout << std::endl;
}

int32_t Segment::GetInvalidBlockCount() const { return invalid_block_count_; }

}  // namespace logstore