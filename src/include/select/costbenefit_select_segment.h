#pragma once

#include "select/select_segment.h"

namespace logstore {

class CostBenefitSelectSegment : public SelectSegment {
 public:
  virtual uint32_t do_select(const std::list<Segment>::iterator &begin,
                             const std::list<Segment>::iterator &end) override;
};

class CostBenefitSelectSegmentFactory : public SelectSegmentFactory {
 public:
  std::unique_ptr<SelectSegment> Create() override;
};

}  // namespace logstore