#pragma once

#include <list>
#include <memory>
#include <vector>
#include "segment/segment.h"
#include "select/select_segment.h"

namespace logstore {

class SegmentManager {
 public:
  SegmentManager(uint32_t segment_num, uint32_t segment_capacity);

  SegmentManager() = delete;
  virtual ~SegmentManager();
  DISALLOW_COPY_AND_MOVE(SegmentManager);

  segment_id_t PBA2SegmentId(pba_t pba) const;
  off64_t PBA2SegmentOffset(pba_t pba) const;
  pba_t AllocateFreeBlock();
  void MarkBlockInvalid(pba_t pba);
  void MarkBlockValid(pba_t pba, lba_t lba);
  bool IsValid(pba_t pba);
  double GetFreeSegmentRatio() const;

  Segment *GetSegment(uint32_t segment_id);
  Segment *FindSegment(pba_t pba);
  std::list<Segment *> &GetOpenedSegments() { return opened_segments_; }
  std::list<Segment *> &GetSealedSegments() { return sealed_segments_; }
  std::list<Segment *> &GetFreeSegments() { return free_segments_; }

  void SetSelectStrategy(std::shared_ptr<SelectSegment> select);

  Segment *SelectVictimSegment();
  Segment *AllocateFreeSegment();

  void DoGCLeftWork(Segment *victim);
  void PrintSegmentsInfo() const;

 private:
  uint32_t segment_num_;
  uint32_t segment_capacity_;
  Segment *segments_;

  std::list<Segment *> opened_segments_;
  std::list<Segment *> sealed_segments_;
  std::list<Segment *> free_segments_;

  std::shared_ptr<SelectSegment> select_;
};

}  // namespace logstore