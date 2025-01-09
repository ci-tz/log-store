#pragma once

#include <memory>
#include <vector>
#include "common/config.h"
#include "common/macros.h"
#include "common/rwlatch.h"

namespace logstore {

class Segment {
 public:
  // Constructor
  Segment(seg_id_t id, pba_t s_pba, int32_t capacity);

  Segment() = default;

  void InitSegment(uint64_t timestamp, int32_t group_id);

  pba_t AppendBlock(lba_t lba);

  void MarkBlockInvalid(off64_t offset);

  bool IsValid(off64_t offset) const;

  bool IsFull() const;

  void SetSealed(bool sealed) { sealed_ = sealed; }

  bool IsSealed() const { return sealed_; }

  seg_id_t GetSegmentId() const;

  int32_t GetCapacity() const;

  uint64_t GetCreateTime() const;

  double GetGarbageRatio() const;

  int32_t GetGroupID() const;

  /**
   * @brief 获取当前segment中的block数量，包括有效的和无效的block数量。
   */
  int32_t Size() const;

  pba_t GetStartPBA() const;

  lba_t GetLBA(off64_t offset) const;

  pba_t GetPBA(off64_t offset) const;

  int32_t GetInvalidBlockCount() const;

  void SetGroupID(int32_t group_id);

  void SetCreateTime(uint64_t create_time);

  void PrintSegmentInfo() const;

 private:
  seg_id_t id_ = 0;       // 与物理segment的ID相同
  pba_t spba_ = 0;        // The physical block address of the first block
  int32_t capacity_ = 0;  // The max number of blocks in the segment

  std::unique_ptr<lba_t[]> rmap_;   // offset -> lba
  uint64_t create_timestamp_ = 0;   // when the first block is appended
  int32_t group_id_ = 0;            // 0,1,2,3...
  int32_t next_append_offset_ = 0;  // write pointer
  int32_t ibc_ = 0;                 // Invalid block count of this segment
  bool sealed_ = false;             // Whether the segment is sealed

  std::vector<std::shared_ptr<Segment>> sub_segments_;  // Only used when group_id_ > 0
};

}  // namespace logstore