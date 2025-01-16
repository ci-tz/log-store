#pragma once

#include <iostream>
#include <memory>

#include "common/config.h"
#include "framework/segment.h"

namespace logstore {

using SegmentVector = std::vector<std::shared_ptr<Segment>>;

class PhySegment {
 public:
  PhySegment(seg_id_t seg_id, pba_t spba, int32_t capacity);
  void OpenAs(int32_t class_id);
  void UpdateRmapValid(pba_t pba, lba_t lba);
  void UpdateRmapInvalid(pba_t pba);

  // Getter
  inline seg_id_t GetSegId() const { return seg_id_; }
  inline int32_t GetCapacity() const { return capacity_; }
  inline pba_t GetSpba() const { return spba_; }
  inline int32_t GetGp() const { return ibc_ * 1.0 / (ibc_ + vbc_); }
  inline bool IsFree() const { return free_; }
  SegmentVector &GetSubSegments() { return sub_segs_; }
  inline lba_t GetLba(pba_t pba) const;
  std::shared_ptr<Segment> GetSegment(pba_t pba);

  // Setter
  inline void SetClassId(int32_t class_id) { class_id_ = class_id; }
  inline void SetFree(bool free) { free_ = free; }

  /**
   * @brief 获取本物理segment被划分成多少个子segment
   */
  inline int32_t GetSubNum() { return 1 << class_id_; }

  friend std::ostream &operator<<(std::ostream &os, const PhySegment &seg);
  friend class Segment;

 private:
  seg_id_t seg_id_ = 0;   // segment 的物理ID
  pba_t spba_ = 0;        // segment的物理起始块地址
  int32_t capacity_ = 0;  // 一个物理segment的容量

  bool free_ = true;  // 本物理segment是否空闲
  int32_t class_id_ = 0;  // 本物理segment所属的热度类，决定了本segment被划分成多少个子segment
  int32_t ibc_ = 0;                // 本物理segment上无效块的总量
  int32_t vbc_ = 0;                // 本物理segment上有效块的总量
  std::unique_ptr<lba_t[]> rmap_;  // 记录本物理segment上每个物理块对应的LBA

  SegmentVector sub_segs_;  // 本物理segment被划分成的子segment
};

};  // namespace logstore