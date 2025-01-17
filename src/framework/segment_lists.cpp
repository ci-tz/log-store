
#include "framework/segment_lists.h"
#include "common/macros.h"

namespace logstore {

void SegmentLists::AddSegment(std::shared_ptr<Segment> segment, int32_t wp) {
  SegmentList &list = segment_lists_[wp];
  list.push_back(segment);
}

std::shared_ptr<Segment> SegmentLists::GetSegment(int32_t wp) {
  LOGSTORE_ASSERT(segment_lists_.find(wp) != segment_lists_.end(), "Invalid List wp");
  SegmentList &list = segment_lists_[wp];
  if (list.empty()) {
    return nullptr;
  }
  auto segment = list.front();
  list.pop_front();
  return segment;
}

bool SegmentLists::IsEmpty(int32_t wp) {
  LOGSTORE_ASSERT(segment_lists_.find(wp) != segment_lists_.end(), "Invalid List wp");
  return segment_lists_[wp].empty();
}

SegmentList &SegmentLists::GetSegmentList(int32_t wp) {
  LOGSTORE_ASSERT(segment_lists_.find(wp) != segment_lists_.end(), "Invalid List wp");
  return segment_lists_[wp];
}

};  // namespace logstore