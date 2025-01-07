#pragma once

#include <list>
#include "select/selection.h"

namespace logstore {

class CostBenefitSelection : public Selection {
 public:
  virtual seg_id_t Select(std::unordered_set<std::shared_ptr<Segment>> &sealed_segments) override;
};

}  // namespace logstore