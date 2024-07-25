#pragma once

#include <list>
#include <memory>
#include <vector>
#include "segment/sealed_segment_list.h"
#include "segment/segment.h"

namespace logstore {

class SegmentManager {
 public:
  SegmentManager(uint32_t segment_num, uint32_t segment_capacity, uint32_t group_num);

  SegmentManager() = delete;
  virtual ~SegmentManager();
  DISALLOW_COPY_AND_MOVE(SegmentManager);

  Segment *FindSegment(pba_t pba);
  uint32_t PBA2SegmentId(pba_t pba);
  off64_t PBA2SegmentOffset(pba_t pba);

  pba_t GetFreeBlock(uint32_t group_id);
  void MarkPBAInvalid(pba_t pba);
  void MarkPBAValid(pba_t pba, lba_t lba);

 private:
  Segment *GetSegment(uint32_t segment_id);
  Segment *GetFreeSegment();
  uint32_t segment_num_;
  uint32_t segment_capacity_;
  Segment *segments_;

  std::vector<Segment *> opened_segments_;
  std::unique_ptr<SealedSegmentList> sealed_segments_;
  std::list<Segment *> free_segments_;
};

}  // namespace logstore