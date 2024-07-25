#include "segment/sealed_segment_list.h"

namespace logstore {

SealedSegmentList::SealedSegmentList(uint32_t group_num) : group_num_(group_num) {
  LOGSTORE_ASSERT(group_num_ > 0, "group_num must be greater than 0");
  segments_.resize(group_num_);
}

void SealedSegmentList::AddSegment(Segment *segment, uint32_t group_id) {
  LOGSTORE_ASSERT(group_id < group_num_ && group_id > 0, "Invalid group id");
  segments_[group_id].push_back(segment);
}

void SealedSegmentList::RemoveSegment(Segment *segment, uint32_t group_id) {
  LOGSTORE_ASSERT(group_id < group_num_ && group_id > 0, "Invalid group id");
  segments_[group_id].remove(segment);
}

std::list<Segment *> &SealedSegmentList::GetSegmentGroup(uint32_t group_id) {
  LOGSTORE_ASSERT(group_id < group_num_ && group_id > 0, "Invalid group id");
  return segments_[group_id];
}

}  // namespace logstore