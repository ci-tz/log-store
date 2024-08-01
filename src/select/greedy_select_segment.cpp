#include "select/greedy_select_segment.h"
#include "framework/segment.h"

#include <algorithm>
#include <cstdint>
#include <list>
#include <map>
#include <vector>

namespace logstore {

seg_id_t GreedySelectSegment::do_select(const std::list<seg_id_t> &sealed_segments,
                                        const Segment *segments) {
  std::vector<std::pair<uint32_t, double>> tmp;
  for (auto it = sealed_segments.begin(); it != sealed_segments.end(); ++it) {
    const Segment *segment = segments + *it;
    tmp.push_back({segment->GetSegmentId(), segment->GetGarbageRatio()});
  }

  // Sort by garbage ratio in descending order
  auto sort_by_garbage_ratio = [](const std::pair<uint32_t, double> &a,
                                  const std::pair<uint32_t, double> &b) {
    return a.second > b.second;
  };
  std::sort(tmp.begin(), tmp.end(), sort_by_garbage_ratio);
  return tmp[0].first;  // Return the segment with the highest garbage ratio
}

std::string GreedySelectSegment::name() const { return "Greedy"; }

}  // namespace logstore