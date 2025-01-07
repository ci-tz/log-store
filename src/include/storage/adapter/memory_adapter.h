#pragma once

#include "storage/adapter/adapter.h"
#include "storage/disk/disk_manager_memory.h"

namespace logstore {

class MemoryAdapter : public Adapter {
 public:
  MemoryAdapter(int32_t num, int32_t capacity);
  ~MemoryAdapter() override = default;

  uint64_t WriteBlock(const char *buf, pba_t pba) override;

  uint64_t ReadBlock(char *buf, pba_t pba) override;

 private:
  DiskManagerMemory disk_manager_memory_;
};

}  // namespace logstore