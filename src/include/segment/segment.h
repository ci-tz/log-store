#pragma once

#include <memory>
#include "common/config.h"
#include "common/macros.h"
#include "common/rwlatch.h"

namespace logstore {

// The interface is not thread-safe
class Segment {
 public:
  Segment() = default;

  // Constructor
  Segment(seg_id_t segment_id, pba_t s_pba, uint32_t capacity);
  DISALLOW_COPY_AND_MOVE(Segment);

  void Init(seg_id_t segment_id, pba_t s_pba, uint32_t capacity);
  void ClearMetadata();

  pba_t AllocateFreeBlock();
  void MarkBlockValid(off64_t offset, lba_t lba);
  void MarkBlockInvalid(off64_t offset);
  bool IsValid(off64_t offset) const;
  bool IsFull() const;

  seg_id_t GetSegmentId() const;
  int32_t GetCapacity() const;
  uint64_t GetCreateTime() const;
  double GetGarbageRatio() const;
  int32_t GetGroupID() const;
  int32_t Size() const;
  pba_t GetStartPBA() const;
  lba_t GetLBA(off64_t offset) const;
  pba_t GetPBA(off64_t offset) const;

  void SetGroupID(int32_t group_id);
  void SetCreateTime(uint64_t create_time);

  void PrintSegmentInfo() const;

  inline void RLatch() { latch_.RLock(); }
  inline void RUnlatch() { latch_.RUnlock(); }
  inline void WLatch() { latch_.WLock(); }
  inline void WUnlatch() { latch_.WUnlock(); }

 private:
  seg_id_t segment_id_ = 0;          // The segment id
  pba_t s_pba_ = 0;                  // The physical block address of the first block
  int32_t capacity_ = 0;             // The max number of blocks in the segment
  std::unique_ptr<lba_t[]> rmap_;    // offset -> lba
  uint64_t create_timestamp_ = 0;    // when the first block is appended
  int32_t group_id_ = -1;            // The group number of the segment, the less if the hotter
  int32_t next_append_offset_ = 0;   // write pointer
  int32_t invalid_block_count_ = 0;  // garbage block count
  ReaderWriterLatch latch_;          // Latch for the segment
};

}  // namespace logstore