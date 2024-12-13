#pragma once

#include "storage/adapter/adapter.h"
#include "storage/disk/disk_manager_memory.h"

namespace logstore {

class MemoryAdapter : public Adapter {
 public:
  MemoryAdapter(int32_t num, int32_t capacity);
  ~MemoryAdapter() override = default;

  void WriteBlock(const char *buf, int32_t id, off64_t offset) override;

  void ReadBlock(char *buf, int32_t id, off64_t offset) override;

 private:
  DiskManagerMemory disk_manager_memory_;
};

}  // namespace logstore