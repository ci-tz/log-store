#include "select/greedy_selection.h"
#include "framework/segment.h"

#include <algorithm>
#include <cstdint>
#include <list>
#include <map>
#include <vector>

namespace logstore {

seg_id_t GreedySelection::Select(std::unordered_set<std::shared_ptr<Segment>> &sealed_segments) {
  std::vector<std::pair<seg_id_t, double>> tmp;  // (segment_id, garbage_ratio)

  for (auto it = sealed_segments.begin(); it != sealed_segments.end(); ++it) {
    std::shared_ptr<Segment> ptr = *it;
    tmp.push_back({ptr->GetSegmentId(), ptr->GetGarbageRatio()});
  }

  // Sort by garbage ratio in descending order
  auto sort_by_garbage_ratio = [](const std::pair<seg_id_t, double> &a,
                                  const std::pair<seg_id_t, double> &b) {
    if (a.second != b.second) {
      return a.second > b.second;
    }
    return a.first < b.first;
  };

  std::sort(tmp.begin(), tmp.end(), sort_by_garbage_ratio);
  if (tmp.empty()) {
    return -1;
  }
  return tmp[0].first;
}

}  // namespace logstore