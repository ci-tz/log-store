#include "select/costbenefit_select_segment.h"
#include "segment/segment.h"

#include <algorithm>
#include <cstdint>
#include <list>
#include <map>
#include <vector>

namespace logstore {

uint32_t CostBenefitSelectSegment::do_select(const std::list<Segment>::iterator &begin,
                                             const std::list<Segment>::iterator &end) {
  std::vector<std::pair<uint32_t, double>> tmp;

  for (auto it = begin; it != end; ++it) {
    uint64_t age =0;
    // uint64_t age = it->GetAge(); // TODO: Implement GetAge() in Segment
    double utilization = 1 - it->GetGarbageRatio();
    double score = utilization / (age * (1 - utilization));
    tmp.push_back({it->GetSegmentId(), score});
  }

  // Sort by score in ascending order
  auto sort_by_score = [](const std::pair<uint32_t, double> &a,
                          const std::pair<uint32_t, double> &b) { return a.second < b.second; };

  std::sort(tmp.begin(), tmp.end(), sort_by_score);
  return tmp[0].first;  // Return the segment with the lowest score
};

std::unique_ptr<SelectSegment> CostBenefitSelectSegmentFactory::Create() {
  return std::make_unique<CostBenefitSelectSegment>();
}

}  // namespace logstore