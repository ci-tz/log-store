#pragma once

#include "select/select_segment.h"

namespace logstore {

class GreedySelectSegment : public SelectSegment {
 public:
  virtual seg_id_t do_select(const std::list<seg_id_t> &sealed_segments,
                             const Segment *segments) override;
  virtual std::string name() const override;
};

}  // namespace logstore