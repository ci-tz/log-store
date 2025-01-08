#pragma once

#include "placement/placement.h"

namespace logstore {

class NoPlacement : public Placement {
 public:
  virtual ~NoPlacement() = default;
  virtual int32_t Classify(lba_t lba, bool is_gc_write) override;
  virtual void MarkUserAppend(lba_t lba, uint64_t timestamp) override;
  virtual void MarkGcAppend(lba_t lba) override;
  virtual void MarkCollectSegment(std::shared_ptr<Segment> ptr) override;
};

};  // namespace logstore