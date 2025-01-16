#include <cstring>
#include <iostream>

#include "common/config.h"
#include "framework/segment.h"
#include "framework/segment_manager.h"

namespace logstore {

Segment::Segment(std::shared_ptr<PhySegment> phy_segment, int32_t class_id, int32_t sub_id)
    : phy_segment_(phy_segment), class_id_(class_id), sub_id_(sub_id) {
  phy_id_ = phy_segment_->GetSegId();
  capacity_ = phy_segment_->GetCapacity() / phy_segment_->GetSubNum();
  spba_ = phy_segment_->GetSpba() + sub_id_ * capacity_;
}

void Segment::InitSegment(uint64_t timestamp, int32_t level_id) {
  level_id_ = level_id;
  next_append_offset_ = 0;
  create_timestamp_ = timestamp;
  ibc_ = 0;
  sealed_ = false;
}

pba_t Segment::AppendBlock(lba_t lba) {
  LOGSTORE_ASSERT(!IsFull(), "Segment is full");
  pba_t pba = spba_ + next_append_offset_;
  MarkBlockValid(pba, lba);
  next_append_offset_++;
  return pba;
}

void Segment::MarkBlockValid(pba_t pba, lba_t lba) {
  LOGSTORE_ASSERT(pba >= spba_, "Invalid pba");
  phy_segment_->UpdateRmapValid(pba, lba);
}

void Segment::MarkBlockInvalid(pba_t pba) {
  LOGSTORE_ASSERT(pba >= spba_, "Invalid pba");
  phy_segment_->UpdateRmapInvalid(pba);
}

uint64_t Segment::GetCreateTime() const { return create_timestamp_; }

pba_t Segment::GetPBA(int32_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  return spba_ + offset;
}

lba_t Segment::GetLBA(int32_t offset) const {
  LOGSTORE_ASSERT(offset < capacity_, "Invalid block index");
  pba_t pba = spba_ + offset;
  return phy_segment_->GetLba(pba);
}

pba_t Segment::GetPBA(int32_t offset) const { return spba_ + offset; }

void Segment::EraseSegment() {
  create_timestamp_ = 0;
  level_id_ = 0;
  next_append_offset_ = 0;
  ibc_ = 0;
  sealed_ = false;

  for (pba_t pba = spba_; pba < spba_ + capacity_; ++pba) {
    MarkBlockInvalid(pba);
  }
}

std::ostream &operator<<(std::ostream &os, const Segment &seg) {
  os << "ClassID: " << seg.class_id_;
  os << ", SubID: " << seg.sub_id_;
  os << ", PhyID: " << seg.phy_id_;
  os << ", Capacity: " << seg.capacity_;
  os << ", Level ID: " << seg.level_id_;
  os << ", SPBA: " << seg.spba_;
  os << ", Next Append Offset: " << seg.next_append_offset_;
  os << ", Create Time: " << seg.create_timestamp_;
  os << ", IBC: " << seg.ibc_;
}

}  // namespace logstore