#pragma once
#include <cstdint>

#include <memory>
#include "common/rwlatch.h"
#include "type/type.h"

namespace logstore {

/** The interface is thread-safe. */
class Segment {
 public:
  Segment(uint32_t segment_id, uint32_t create_timestamp, uint32_t max_size_);

  // Disallow default constructor
  Segment() = delete;

  // Disallow copy and assign
  Segment(const Segment &) = delete;
  Segment &operator=(const Segment &) = delete;

  // Destructor
  virtual ~Segment() = default;

  pba_t Append(lba_t lba);
  void MarkInvalid(uint32_t idx);

  bool IsBlockValid(uint32_t idx);
  bool IsFull();

  uint64_t GetAge() const;
  uint32_t GetSegmentId() const;
  uint32_t GetMaxSize() const;

  double GetGarbageRatio();
  uint32_t GetClassNum();
  uint32_t GetNextAppendOffset();

  bool IsSealed();
  void SetSealed();

  void SetClass(uint32_t class_num);

 private:
  inline void RLatch() { latch_.RLock(); }
  inline void RUnlatch() { latch_.RUnlock(); }
  inline void WLatch() { latch_.WLock(); }
  inline void WUnlatch() { latch_.WUnlock(); }

  const uint32_t segment_id_;
  const uint64_t create_timestamp_;
  const uint32_t max_size_;

  uint32_t class_num_ = 0;
  uint32_t next_append_offset_ = 0;
  bool sealed_ = false;
  std::unique_ptr<lba_t[]> rmap_;  // offset -> lba
  uint32_t invalid_block_count_ = 0;
  ReaderWriterLatch latch_;  // Latch for the segment
};

}  // namespace logstore