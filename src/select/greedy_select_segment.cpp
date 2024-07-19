#include "select/greedy_select_segment.h"
#include "logstore/segment.h"

#include <algorithm>
#include <cstdint>
#include <list>
#include <map>
#include <vector>

namespace logstore {

uint32_t GreedySelectSegment::do_select(const std::list<Segment>::iterator &begin,
                                        const std::list<Segment>::iterator &end) {
  std::vector<std::pair<uint32_t, double>> tmp;
  for (auto it = begin; it != end; ++it) {
    tmp.push_back({it->GetSegmentId(), it->GetGarbageRatio()});
  }

  // Sort by garbage ratio in descending order
  auto sort_by_garbage_ratio = [](const std::pair<uint32_t, double> &a,
                                  const std::pair<uint32_t, double> &b) {
    return a.second > b.second;
  };

  std::sort(tmp.begin(), tmp.end(), sort_by_garbage_ratio);
  return tmp[0].first;  // Return the segment with the highest garbage ratio
}

std::unique_ptr<SelectSegment> GreedySelectSegmentFactory::Create() {
  return std::make_unique<GreedySelectSegment>();
}

}  // namespace logstore