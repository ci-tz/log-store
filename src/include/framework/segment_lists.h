#pragma once

#include <list>
#include <memory>
#include <unordered_map>

#include "framework/segment.h"

namespace logstore {

using SegmentList = std::list<std::shared_ptr<Segment>>;

class SegmentLists {
 public:
  void AddSegment(std::shared_ptr<Segment> segment, int32_t wp);
  std::shared_ptr<Segment> GetSegment(int32_t wp);
  bool IsEmpty(int32_t wp);
  SegmentList &GetSegmentList(int32_t wp);

 private:
  std::unordered_map<int32_t, SegmentList> segment_lists_;
};

};  // namespace logstore