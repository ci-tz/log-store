#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>
#include "framework/segment.h"
#include "select/select_segment.h"

namespace logstore {

class SegmentManager {
 public:
  SegmentManager(int32_t segment_num, int32_t segment_capacity,
                 std::shared_ptr<SelectSegment> select);
  virtual ~SegmentManager();
  DISALLOW_COPY_AND_MOVE(SegmentManager);

  seg_id_t GetSegmentId(pba_t pba) const;
  off64_t GetOffset(pba_t pba) const;
  Segment *GetSegment(seg_id_t segment_id);
  Segment *FindSegment(pba_t pba);

  pba_t AllocateFreeBlock();
  void MarkBlockInvalid(pba_t pba);
  void MarkBlockValid(pba_t pba, lba_t lba);
  bool IsValid(pba_t pba);
  double GetFreeSegmentRatio() const;
  int32_t GetInvalidBlockNum() const;

  void SetSelectStrategy(std::shared_ptr<SelectSegment> select);
  seg_id_t SelectVictimSegment();
  seg_id_t AllocateFreeSegment();
  void DoGCLeftWork(seg_id_t victim_segment_id);

  void PrintSegmentsInfo();

  std::list<seg_id_t> &GetOpenedSegments() { return opened_segments_; }
  std::list<seg_id_t> &GetSealedSegments() { return sealed_segments_; }
  std::list<seg_id_t> &GetFreeSegments() { return free_segments_; }

 private:
  int32_t segment_num_;
  int32_t segment_capacity_;
  int32_t total_block_num_ = 0;
  int32_t total_invalid_block_num_ = 0;
  Segment *segments_;

  std::list<seg_id_t> opened_segments_;
  std::list<seg_id_t> sealed_segments_;
  std::list<seg_id_t> free_segments_;

  std::shared_ptr<SelectSegment> select_;
};

}  // namespace logstore