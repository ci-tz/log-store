#pragma once

#include <list>
#include <memory>
#include <unordered_map>

#include "framework/segment.h"

namespace logstore {

using SegmentList = std::list<std::shared_ptr<Segment>>;

class SegmentLists {
 public:
  void AddSegment(int32_t id, std::shared_ptr<Segment> segment);
  std::shared_ptr<Segment> GetSegment(int32_t id);
  SegmentList &GetSegmentList(int32_t id);

 private:
  std::unordered_map<int32_t, SegmentList> segment_lists_;
}

};  // namespace logstore