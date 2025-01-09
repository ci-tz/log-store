#pragma once

#include "storage/adapter/adapter.h"
#include "storage/disk/disk_manager_memory.h"

namespace logstore {

class MemoryAdapter : public Adapter {
 public:
  MemoryAdapter(int32_t num, int32_t capacity);
  ~MemoryAdapter() override = default;

  virtual uint64_t WriteBlock(const char *buf, pba_t pba) override;

  virtual uint64_t ReadBlock(char *buf, pba_t pba) override;

  virtual void EraseSegment(seg_id_t seg_id) override;

 private:
  DiskManagerMemory disk_manager_memory_;
};

}  // namespace logstore