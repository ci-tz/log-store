#pragma once

#include "segment/segment.h"

#include <cstdint>
#include <list>

namespace logstore {

class SelectSegment {
 public:
  virtual ~SelectSegment() = default;

  virtual uint32_t do_select(const std::list<Segment>::iterator &begin,
                             const std::list<Segment>::iterator &end) = 0;
};

class SelectSegmentFactory {
 public:
  virtual ~SelectSegmentFactory() = default;

  virtual std::unique_ptr<SelectSegment> Create() = 0;
};

}  // namespace logstore