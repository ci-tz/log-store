#pragma once

#include <memory>

#include "common/config.h"
#include "framework/segment.h"

namespace logstore {

class Placement {
 public:
  virtual ~Placement() = default;
  virtual int Classify(lba_t lba, bool is_gc_write) = 0;
  virtual void MarkUserAppend(lba_t lba, uint64_t timestamp) = 0;
  virtual void MarkGcAppend(lba_t lba) = 0;
  virtual void MarkCollectSegment(std::shared_ptr<Segment> ptr) = 0;
};

};  // namespace logstore