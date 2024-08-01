#include "select/costbenefit_select_segment.h"
#include "framework/segment.h"

#include <algorithm>
#include <cstdint>
#include <list>
#include <map>
#include <vector>

namespace logstore {

seg_id_t CostBenefitSelectSegment::do_select(const std::list<seg_id_t> &sealed_segments,
                                             const Segment *segments) {
  std::vector<std::pair<uint32_t, double>> tmp;
  uint64_t current_time = INT64_MAX;  // TODO: Implement GetCurrentTime() in Segment

  for (auto it = sealed_segments.begin(); it != sealed_segments.end(); ++it) {
    const Segment *segment = segments + *it;
    uint64_t age = current_time - segment->GetCreateTime();
    double utilization = 1 - segment->GetGarbageRatio();
    double score = utilization / (age * (1 - utilization));
    tmp.push_back({segment->GetSegmentId(), score});
  }

  // Sort by score in ascending order
  auto sort_by_score = [](const std::pair<uint32_t, double> &a,
                          const std::pair<uint32_t, double> &b) { return a.second < b.second; };

  std::sort(tmp.begin(), tmp.end(), sort_by_score);
  return tmp[0].first;  // Return the segment with the lowest score
};

std::string CostBenefitSelectSegment::name() const { return "CostBenefit"; }

}  // namespace logstore