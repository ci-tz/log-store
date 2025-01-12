#pragma once

#include <unordered_map>
#include <vector>

#include "common/config.h"

namespace logstore {

class GcLifespan {
 public:
  void MarkUserWrite(lba_t lba, int64_t write_time);
  void MarkGc(lba_t lba);
  void PrintCount();
  void PrintAverageLifespan();

 private:
  std::unordered_map<lba_t, int64_t> last_write_time_;   // lba --> last write time
  std::unordered_map<lba_t, int32_t> migrate_count_;     // lba --> migrate count before it is overwritten
  std::unordered_map<int32_t, int32_t> count_;           // migrate count --> count
  std::unordered_map<int32_t, int64_t> total_lifespan_;  // migrate count --> total lifespan
};

};  // namespace logstore