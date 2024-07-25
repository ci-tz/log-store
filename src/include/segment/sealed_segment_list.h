#pragma once

#include <list>
#include <vector>
#include "common/macros.h"
#include "segment/segment.h"

namespace logstore {

class SealedSegmentList {
 public:
  explicit SealedSegmentList(uint32_t group_num = 1);

  virtual ~SealedSegmentList() = default;
  DISALLOW_COPY_AND_MOVE(SealedSegmentList);

  void AddSegment(Segment *segment, uint32_t group_id);
  void RemoveSegment(Segment *segment, uint32_t group_id);

  std::list<Segment *> &GetSegmentGroup(uint32_t group_id);

 private:
  uint32_t group_num_;  // sealed segment is divided into groups
  std::vector<std::list<Segment *>> segments_;
};

}  // namespace logstore