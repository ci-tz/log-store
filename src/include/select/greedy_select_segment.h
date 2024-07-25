#pragma once

#include "select/select_segment.h"

namespace logstore {

class GreedySelectSegment : public SelectSegment {
 public:
  virtual uint32_t do_select() override;
};

class GreedySelectSegmentFactory : public SelectSegmentFactory {
 public:
  std::unique_ptr<SelectSegment> Create() override;
};

}  // namespace logstore