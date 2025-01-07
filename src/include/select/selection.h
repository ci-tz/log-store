#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>

#include "framework/segment.h"

namespace logstore {

class Selection {
 public:
  Selection() = default;
  virtual ~Selection() = default;
  virtual seg_id_t Select(std::unordered_set<std::shared_ptr<Segment>> &sealed_segments) = 0;
};

}  // namespace logstore