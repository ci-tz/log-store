#pragma once

#include <memory>
#include "common/config.h"
#include "common/macros.h"
#include "common/rwlatch.h"

namespace logstore {

// The interface is not thread-safe
class Segment {
 public:
  // The invalid offset
  constexpr static off64_t INVALID_OFFSET = -1;

  Segment() = default;

  // Constructor
  Segment(uint32_t segment_id, pba_t s_pba, uint32_t capacity);

  // Destructor
  virtual ~Segment() = default;

  // Disallow copy and move semantics
  DISALLOW_COPY_AND_MOVE(Segment);

  void Init(uint32_t segment_id, pba_t s_pba, uint32_t capacity);
  void Clear();

  pba_t AllocateFreeBlock();
  void MarkBlockValid(off64_t offset, lba_t lba);
  void MarkBlockInvalid(off64_t offset);
  bool IsBlockValid(off64_t offset) const;
  bool IsFull() const;

  uint32_t GetSegmentId() const;
  uint32_t GetCapacity() const;
  uint64_t GetCreateTime() const;
  double GetGarbageRatio() const;
  uint32_t GetGroupID() const;
  uint32_t Size() const;

  bool IsSealed() const;
  void SetGroupID(uint32_t group_id);
  void SetCreateTime(uint64_t create_time);

  inline void RLatch() { latch_.RLock(); }
  inline void RUnlatch() { latch_.RUnlock(); }
  inline void WLatch() { latch_.WLock(); }
  inline void WUnlatch() { latch_.WUnlock(); }

 private:
  uint32_t segment_id_ = 0;           // The segment id
  pba_t s_pba_ = 0;                   // The physical block address of the first block
  uint32_t capacity_ = 0;             // The max number of blocks in the segment
  std::unique_ptr<lba_t[]> rmap_;     // offset -> lba
  uint64_t create_timestamp_ = 0;     // when the first block is appended
  uint32_t group_id_ = -1;            // The group number of the segment, the less if the hotter
  uint32_t next_append_offset_ = 0;   // write pointer
  bool sealed_ = false;               // If the segment is full, it is sealed
  uint32_t invalid_block_count_ = 0;  // garbage block count
  ReaderWriterLatch latch_;           // Latch for the segment
};

}  // namespace logstore