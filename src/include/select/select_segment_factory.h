#pragma once

#include "select/costbenefit_select_segment.h"
#include "select/greedy_select_segment.h"
#include "select/select_segment.h"

namespace logstore {

class SelectSegmentFactory {
 public:
  static std::shared_ptr<SelectSegment> CreateSelectSegment(const std::string &type) {
    if (type == "Greedy") {
      return std::make_shared<GreedySelectSegment>();
    } else if (type == "CostBenefit") {
      return std::make_shared<CostBenefitSelectSegment>();
    } else {
      return nullptr;
    }
  }
};

}  // namespace logstore