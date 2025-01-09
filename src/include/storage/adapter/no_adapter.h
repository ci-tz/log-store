#pragma once

#include "storage/adapter/adapter.h"

namespace logstore {

class NoAdapter : public Adapter {
 public:
  NoAdapter(int32_t num, int32_t capacity);
  ~NoAdapter() override = default;

  uint64_t WriteBlock(const char *buf, pba_t pba) override;

  uint64_t ReadBlock(char *buf, pba_t pba) override;
};

}  // namespace logstore