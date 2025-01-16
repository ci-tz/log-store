
#include "framework/segment_lists.h"
#include "common/macros.h"

namespace logstore {

void SegmentLists::AddSegment(int32_t id, std::shared_ptr<Segment> segment) {
  SegmentList &list = segment_lists_[id];
  list.push_back(segment);
}

std::shared_ptr<Segment> SegmentLists::GetSegment(int32_t id) {
  LOGSTORE_ASSERT(segment_lists_.find(id) != segment_lists_.end(), "Invalid List Id");
  SegmentList &list = segment_lists_[id];
  if (list.empty()) {
    return nullptr;
  }
  auto segment = list.front();
  list.pop_front();
  return segment;
}

SegmentList &SegmentLists::GetSegmentList(int32_t id) {
  LOGSTORE_ASSERT(segment_lists_.find(id) != segment_lists_.end(), "Invalid List Id");
  return segment_lists_[id];
}

};  // namespace logstore