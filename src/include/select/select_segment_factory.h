#pragma once

#include "select/costbenefit_select_segment.h"
#include "select/greedy_select_segment.h"
#include "select/select_segment.h"

namespace logstore {

class SelectSegmentFactory {
 public:
  static std::shared_ptr<SelectSegment> CreateSelectSegment(const std::string &select_strategy) {
    if (select_strategy == "Greedy") {
      return std::make_shared<GreedySelectSegment>();
    } else if (select_strategy == "CostBenefit") {
      return std::make_shared<CostBenefitSelectSegment>();
    } else {
      return nullptr;
    }
  }
};

}  // namespace logstore