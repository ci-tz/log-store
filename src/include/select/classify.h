#pragma once

#include "common/config.h"

namespace logstore {

class Classify {
 public:
  virtual ~Classify() = default;
  virtual uint32_t classify_usr_write(lba_t lba) = 0;
  virtual uint32_t classify_gc_write(lba_t lba) = 0;
  virtual void update_write(lba_t lba, uint64_t timestamp) = 0;
  virtual void update_gc(lba_t lba, uint64_t timestamp) = 0;
};

}  // namespace logstore