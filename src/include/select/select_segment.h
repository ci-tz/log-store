#pragma once

#include "segment/segment.h"

#include <cstdint>
#include <list>
#include <string>

namespace logstore {

class SelectSegment {
 public:
  virtual ~SelectSegment() = default;
  virtual uint32_t do_select(const std::list<Segment *>::iterator &begin,
                             const std::list<Segment *>::iterator &end) = 0;
  virtual std::string name() const = 0;
};

}  // namespace logstore