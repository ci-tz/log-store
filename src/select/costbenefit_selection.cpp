#include "select/costbenefit_selection.h"
#include "framework/segment.h"
#include "framework/segment_manager.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace logstore {

seg_id_t CostBenefitSelection::Select(std::unordered_set<std::shared_ptr<Segment>> &sealed_segments) {
  std::vector<std::pair<seg_id_t, double>> tmp;  // (segment_id, score)
  uint64_t current_time = SegmentManager::write_timestamp;

  for (auto it = sealed_segments.begin(); it != sealed_segments.end(); ++it) {
    std::shared_ptr<Segment> ptr = *it;
    double gp = ptr->GetGarbageRatio();
    double age = current_time - ptr->GetCreateTime();
    double score = (gp == 1.0) ? DBL_MAX : gp / (1 - gp) * sqrt(age);
    tmp.push_back({ptr->GetSegmentId(), score});
  }

  // Sort by score in descending order
  auto sort_by_score = [](const std::pair<seg_id_t, double> &a, const std::pair<seg_id_t, double> &b) {
    return a.second > b.second;
  };

  std::sort(tmp.begin(), tmp.end(), sort_by_score);
  if (tmp.empty()) {
    return -1;
  }
  return tmp[0].first;
};

}  // namespace logstore