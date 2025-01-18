#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "common/config.h"
#include "common/macros.h"
#include "framework/phy_segment.h"

namespace logstore {

class PhySegment;

class Segment {
 public:
  // Constructor
  Segment() = default;
  Segment(PhySegment *phy_segment, int32_t class_id, int32_t sub_id);

  void InitSegment(uint64_t timestamp, int32_t level);
  void EraseSegment();
  pba_t AppendBlock(lba_t lba);
  void MarkBlockInvalid(pba_t pba);
  void MarkBlockValid(pba_t pba, lba_t lba);

  // Getters
  inline bool IsFull() const { return Size() == capacity_; }
  inline int32_t Size() const { return next_append_offset_; }
  inline bool IsSealed() const { return sealed_; }
  inline int32_t GetSubId() const { return sub_id_; }
  inline int32_t GetCapacity() const { return capacity_; }
  inline uint64_t GetCreateTime() const { return create_timestamp_; }
  inline int32_t GetClassID() const { return class_id_; }
  inline pba_t GetStartPBA() const { return spba_; }
  inline double GetGarbageRatio() const { return ibc_ * 1.0 / Size(); }
  inline int32_t GetLevelID() const { return level_id_; }
  // inline uint64_t GetAge() const { return SegmentManager::write_timestamp - create_timestamp_; }
  lba_t GetLBA(int32_t offset) const;
  pba_t GetPBA(int32_t offset) const;
  int32_t GetIBC() const { return ibc_; }

  // Setters
  inline void SetSealed(bool sealed) { sealed_ = sealed; }
  inline void SetCreateTime(uint64_t create_time) { create_timestamp_ = create_time; }

  friend std::ostream &operator<<(std::ostream &os, const Segment &seg);

 private:
  int32_t class_id_ = 0;     // 该逻辑segment所属的热度等级
  int32_t sub_id_ = 0;       // sub ID，一个物理segment可以包含多个逻辑segment
  PhySegment *phy_segment_;  // 所属的物理segment的指针
  seg_id_t phy_id_ = 0;      // 所属物理segment的ID
  int32_t capacity_ = 0;     // 该逻辑segment的容量，即最多可以包含多少个block
  pba_t spba_ = 0;           // 该逻辑segment的开始PBA地址

  int32_t level_id_ = 0;            // 该逻辑segment所属的层级
  int32_t next_append_offset_ = 0;  // 写指针
  uint64_t create_timestamp_ = 0;   // when the first block is appended
  int32_t ibc_ = 0;                 // Invalid block count of this segment
  bool sealed_ = false;             // Whether the segment is sealed
};

}  // namespace logstore