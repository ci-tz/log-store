#include <iostream>

#include "common/config.h"
#include "framework/segment.h"

namespace logstore {

Segment::Segment(seg_id_t id, pba_t s_pba, int32_t capacity) : id_(id), spba_(s_pba), capacity_(capacity) {
  rmap_ = std::make_unique<lba_t[]>(capacity_);
  for (int32_t i = 0; i < capacity_; i++) {
    rmap_[i] = INVALID_LBA;
  }
}

void Segment::InitSegment(uint64_t timestamp, int32_t group_id) {
  create_timestamp_ = timestamp;
  group_id_ = group_id;

  for (int32_t i = 0; i < capacity_; i++) {
    rmap_[i] = INVALID_LBA;
  }
  next_append_offset_ = 0;
  ibc_ = 0;
  sealed_ = false;
}

pba_t Segment::AppendBlock(lba_t lba) {
  if (IsFull()) {
    return INVALID_PBA;
  }
  pba_t pba = spba_ + next_append_offset_;
  rmap_[next_append_offset_] = lba;
  next_append_offset_++;
  return pba;
}

void Segment::MarkBlockInvalid(off64_t offset) {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  LOGSTORE_ASSERT(rmap_[offset] != INVALID_LBA, "Block is already invalid");
  rmap_[offset] = INVALID_LBA;
  ibc_++;
}

bool Segment::IsValid(off64_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  return rmap_[offset] != INVALID_LBA;
}

bool Segment::IsFull() const { return Size() == capacity_; }

double Segment::GetGarbageRatio() const { return static_cast<double>(ibc_) / Size(); }

uint64_t Segment::GetCreateTime() const { return create_timestamp_; }

seg_id_t Segment::GetSegmentId() const { return id_; }

int32_t Segment::GetGroupID() const { return group_id_; }

int32_t Segment::GetCapacity() const { return capacity_; }

int32_t Segment::Size() const { return next_append_offset_; }

pba_t Segment::GetStartPBA() const { return spba_; }

int32_t Segment::GetInvalidBlockCount() const { return ibc_; }

pba_t Segment::GetPBA(off64_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  return spba_ + offset;
}

lba_t Segment::GetLBA(off64_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  return rmap_[offset];
}

void Segment::SetGroupID(int32_t group_id) { group_id_ = group_id; }

void Segment::SetCreateTime(uint64_t create_time) { create_timestamp_ = create_time; }

void Segment::PrintSegmentInfo() const {
  std::cout << "-------------------" << std::endl;
  std::cout << "Segment ID: " << id_ << std::endl;
  std::cout << "Create Time: " << create_timestamp_ << std::endl;
  std::cout << "Next Append Offset: " << next_append_offset_ << std::endl;
  std::cout << "Invalid Block Count: " << ibc_ << std::endl;
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

}  // namespace logstore