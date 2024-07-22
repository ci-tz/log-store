#include "logstore/segment.h"
#include "common/config.h"

namespace logstore {

Segment::Segment(uint32_t segment_id, pba_t s_pba, uint32_t create_timestamp, uint32_t max_size_)
    : segment_id_(segment_id),
      create_timestamp_(create_timestamp),
      max_size_(max_size_),
      s_pba_(s_pba) {
  rmap_ = std::make_unique<lba_t[]>(max_size_);
  for (uint32_t i = 0; i < max_size_; i++) {
    rmap_[i] = INVALID_LBA;
  }
}

pba_t Segment::Append(lba_t lba) {
  if (IsFull()) {
    throw std::runtime_error("Segment is full");
  }
  pba_t pba = s_pba_ + next_append_offset_;
  rmap_[next_append_offset_] = lba;
  next_append_offset_++;
  return pba;
}

void Segment::MarkInvalid(uint32_t offset) {
  if (rmap_[offset] != INVALID_LBA) {
    invalid_block_count_++;
    rmap_[offset] = INVALID_LBA;
  }
}

bool Segment::IsBlockValid(uint32_t offset) { return rmap_[offset] != INVALID_LBA; }

bool Segment::IsFull() { return next_append_offset_ == max_size_; }

double Segment::GetGarbageRatio() { return static_cast<double>(invalid_block_count_) / max_size_; }

uint64_t Segment::GetAge() {
  return create_timestamp_;  // TODO: implement this function to
}

uint32_t Segment::GetSegmentId() const { return segment_id_; }

uint32_t Segment::GetGroupNum() { return group_num_; }

uint32_t Segment::GetNextAppendOffset() { return next_append_offset_; }

uint32_t Segment::GetMaxSize() const { return max_size_; }

bool Segment::IsSealed() { return sealed_; }

void Segment::SetGroupNum(uint32_t group_num) { group_num_ = group_num; }

void Segment::SetSealed() { sealed_ = true; }

void Segment::SetCreateTimestamp(uint64_t create_timestamp) {
  create_timestamp_ = create_timestamp;
}

}  // namespace logstore