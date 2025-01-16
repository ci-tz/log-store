#include <cstring>

#include "framework/phy_segment.h"
#include "framework/segment.h"

namespace logstore {

PhySegment::PhySegment(seg_id_t seg_id, pba_t spba, int32_t capacity)
    : seg_id_(seg_id), spba_(spba), capacity_(capacity) {
  rmap_ = std::make_unique<lba_t[]>(capacity_);
  memset(rmap_.get(), INVALID_LBA, capacity_ * sizeof(lba_t));
}

void PhySegment::OpenAs(int32_t class_id) {
  class_id_ = class_id;
  int32_t sub_num = GetSubNum();
  for (int32_t i = 0; i < sub_num; i++) {
    auto sub_ptr = std::make_shared<Segment>(this, class_id, i);
    sub_segs_.emplace_back(sub_ptr);
  }
}

void PhySegment::UpdateRmapValid(pba_t pba, lba_t lba) {
  LOGSTORE_ASSERT(pba >= spba_ && pba < spba_ + capacity_, "out of range");
  int32_t offset = pba % capacity_;
  rmap_[offset] = lba;
  vbc_++;
}

void PhySegment::UpdateRmapInvalid(pba_t pba) {
  LOGSTORE_ASSERT(pba >= spba_ && pba < spba_ + capacity_, "out of range");
  int32_t offset = pba % capacity_;
  rmap_[offset] = INVALID_LBA;
  ibc_++;
}

lba_t PhySegment::GetLba(pba_t pba) const {
  LOGSTORE_ASSERT(pba >= spba_ && pba < spba_ + capacity_, "out of range");
  int32_t offset = pba % capacity_;
  return rmap_[offset];
}

std::shared_ptr<Segment> PhySegment::GetSegment(pba_t pba) {
  LOGSTORE_ASSERT(pba >= spba_ && pba < spba_ + capacity_, "out of range");
  int32_t offset = pba % capacity_;
  int32_t sub_num = GetSubNum();
  int32_t sub_cap = capacity_ / sub_num;
  int32_t sub_id = offset / sub_cap;
  LOGSTORE_ASSERT(sub_id < sub_segs_.size(), "sub_id out of range");
  return sub_segs_[sub_id];
}

std::ostream &operator<<(std::ostream &os, const PhySegment &seg) {
  os << "[" << seg.seg_id_ << "]: ";
  os << "spba: " << seg.spba_;
  os << ", capacity: " << seg.capacity_;
  os << ", free:" << seg.free_ ? "T" : "F";

  if (!seg.free_) {
    os << ", class_id: " << seg.class_id_;
    os << ", Invalid Blocks: " << seg.ibc_;
    os << "Rmap: [";
    for (int32_t i = 0; i < seg.capacity_; i++) {
      lba_t lba = seg.GetLba(i);
      if (lba == INVALID_LBA) {
        os << "I";
      } else {
        os << lba;
      }

      if (i == seg.capacity_ - 1) {
        os << "]" << std::endl;
      } else {
        os << ",";
      }
    }
  } else {
    os << std::endl;
  }
}

};  // namespace logstore