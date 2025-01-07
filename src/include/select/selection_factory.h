#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "select/costbenefit_selection.h"
#include "select/greedy_selection.h"
#include "select/selection.h"

namespace logstore {

class SelectionFactory {
 public:
  static std::shared_ptr<Selection> GetSelection(const std::string &type) {
    if (type == "Greedy") {
      return std::make_shared<GreedySelection>();
    } else if (type == "CostBenefit") {
      return std::make_shared<CostBenefitSelection>();
    } else {
      std::cerr << "Unknown selection type: " << type << std::endl;
      return nullptr;
    }
  }
};

}  // namespace logstore