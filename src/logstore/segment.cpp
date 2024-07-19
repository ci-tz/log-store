#include "logstore/segment.h"
#include "common/config.h"

namespace logstore {

Segment::Segment(uint32_t segment_id, uint32_t create_timestamp, uint32_t max_size_)
    : segment_id_(segment_id), create_timestamp_(create_timestamp), max_size_(max_size_) {
  rmap_ = std::make_unique<lba_t[]>(max_size_);
  for (uint32_t i = 0; i < max_size_; i++) {
    rmap_[i] = INVALID_LBA;
  }
}

pba_t Segment::Append(lba_t lba) {
  if (IsFull()) {
    throw std::runtime_error("Segment is full");
  }
  WLatch();
  pba_t pba = segment_id_ * Config::GetInstance().kSegmentCapacity + next_append_offset_;
  rmap_[next_append_offset_] = lba;
  next_append_offset_++;
  WUnlatch();
  return pba;
}

void Segment::MarkInvalid(uint32_t idx) {
  WLatch();
  if (rmap_[idx] != INVALID_LBA) {
    invalid_block_count_++;
    rmap_[idx] = INVALID_LBA;
  }
  WUnlatch();
}

bool Segment::IsBlockValid(uint32_t idx) {
  RLatch();
  bool ret = rmap_[idx] != INVALID_LBA;
  RUnlatch();
  return ret;
}

bool Segment::IsFull() {
  RLatch();
  bool ret = next_append_offset_ == max_size_;
  RUnlatch();
  return ret;
}

double Segment::GetGarbageRatio() {
  RLatch();
  double ret = static_cast<double>(invalid_block_count_) / max_size_;
  RUnlatch();
  return ret;
}

uint64_t Segment::GetAge() const {
  return create_timestamp_;  // TODO: implement this function to
}

uint32_t Segment::GetSegmentId() const { return segment_id_; }

uint32_t Segment::GetClassNum() {
  RLatch();
  uint32_t ret = class_num_;
  RUnlatch();
  return ret;
}

uint32_t Segment::GetNextAppendOffset() {
  RLatch();
  uint32_t ret = next_append_offset_;
  RUnlatch();
  return ret;
}

uint32_t Segment::GetMaxSize() const { return max_size_; }

bool Segment::IsSealed() {
  RLatch();
  bool ret = sealed_;
  RUnlatch();
  return ret;
}

void Segment::SetClass(uint32_t class_num) {
  WLatch();
  class_num_ = class_num;
  WUnlatch();
}

void Segment::SetSealed() {
  WLatch();
  sealed_ = true;
  WUnlatch();
}

}  // namespace logstore