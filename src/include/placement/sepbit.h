#pragma once

#include <cfloat>
#include <memory>
#include <unordered_map>

#include "common/fifo.h"
#include "placement/placement.h"

namespace logstore {

class SepBIT : public Placement {
 public:
  SepBIT();
  virtual ~SepBIT() = default;
  virtual int Classify(lba_t lba, bool is_gc_write) override;
  virtual void MarkUserAppend(lba_t lba, uint64_t timestamp) override;
  virtual void MarkGcAppend(lba_t lba) override;
  virtual void MarkCollectSegment(std::shared_ptr<Segment> ptr) override;

 private:
  std::unique_ptr<FIFO> fifo_;
  std::unordered_map<lba_t, uint64_t> last_write_time_;
  double avg_lifespan_ = DBL_MAX;
  int32_t group_id_of_last_collected_segment_;
};

};  // namespace logstore