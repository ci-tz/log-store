#pragma once

#include "select/selection.h"

namespace logstore {

class GreedySelection : public Selection {
 public:
  virtual seg_id_t Select(std::unordered_set<std::shared_ptr<Segment>> &sealed_segments) override;
};

}  // namespace logstore