#pragma once

#include "framework/segment.h"

#include <cstdint>
#include <list>
#include <string>

namespace logstore {

class SelectSegment {
 public:
  virtual ~SelectSegment() = default;
  virtual seg_id_t do_select(const std::list<seg_id_t> &sealed_segments,
                             const Segment *segments) = 0;
  virtual std::string name() const = 0;
};

}  // namespace logstore