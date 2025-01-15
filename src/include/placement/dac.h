#pragma once

#include <memory>
#include <unordered_map>

#include "index/hash_indexmap.h"
#include "placement/placement.h"

namespace logstore {

class DAC : public Placement {
 public:
  DAC(int32_t group_num) : group_num_(group_num) {}
  virtual ~DAC() override = default;
  virtual int32_t Classify(lba_t lba, bool is_gc_write) override;

  virtual void MarkUserAppend(lba_t lba, uint64_t timestamp) override;

  virtual void MarkGcAppend(lba_t lba) override;

  virtual void MarkCollectSegment(std::shared_ptr<Segment> ptr) override;

 private:
  int32_t group_num_;
  std::unordered_map<lba_t, int32_t> lba_temprature_;  // LBA --> Class ID
};

};  // namespace logstore
