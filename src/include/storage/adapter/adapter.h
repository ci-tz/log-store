#pragma once

#include <vector>
#include "common/config.h"
#include "common/macros.h"

namespace logstore {

class Adapter {
 public:
  explicit Adapter(int32_t segment_num, int32_t segment_capacity)
      : segment_num_(segment_num), segment_capacity_(segment_capacity) {}
  Adapter() = delete;
  virtual ~Adapter() = default;

  virtual void WriteBlock(const char *buf, seg_id_t segment_id, off64_t offset) = 0;
  virtual void ReadBlock(char *buf, seg_id_t segment_id, off64_t offset) = 0;

 protected:
  int32_t segment_num_;       // The number of segments
  int32_t segment_capacity_;  // The number of blocks in a segment
};

}  // namespace logstore