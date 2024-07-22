#pragma once
#include <cstdint>

#include <memory>
#include "common/macros.h"
#include "common/rwlatch.h"
#include "type/type.h"

namespace logstore {

// The interface is not thread-safe
class Segment {
 public:
  Segment(uint32_t segment_id, pba_t s_pba, uint32_t create_timestamp, uint32_t max_size_);

  // Disallow default constructor
  Segment() = delete;

  // Destructor
  virtual ~Segment() = default;

  // Disallow copy and move semantics
  DISALLOW_COPY_AND_MOVE(Segment);

  pba_t Append(lba_t lba);
  void MarkInvalid(uint32_t idx);

  bool IsBlockValid(uint32_t idx);
  bool IsFull();

  uint32_t GetSegmentId() const;
  uint32_t GetMaxSize() const;

  uint64_t GetAge();

  double GetGarbageRatio();
  uint32_t GetGroupNum();
  uint32_t GetNextAppendOffset();

  bool IsSealed();
  void SetSealed();
  void SetGroupNum(uint32_t group_num);
  void SetCreateTimestamp(uint64_t create_timestamp);

  inline void RLatch() { latch_.RLock(); }
  inline void RUnlatch() { latch_.RUnlock(); }
  inline void WLatch() { latch_.WLock(); }
  inline void WUnlatch() { latch_.WUnlock(); }

 private:
  const uint32_t segment_id_;  // The segment id
  const uint32_t max_size_;    // unit: block, must be 2^n
  const pba_t s_pba_;          // The physical block address of the first block
  uint64_t create_timestamp_;  // when the first block is appended

  uint32_t group_num_ = 0;  // The group number of the segment, the less if the hotter
  uint32_t next_append_offset_ = 0;
  bool sealed_ = false;            // If the segment is full, it is sealed
  std::unique_ptr<lba_t[]> rmap_;  // offset -> lba
  uint32_t invalid_block_count_ = 0;
  ReaderWriterLatch latch_;  // Latch for the segment
};

}  // namespace logstore