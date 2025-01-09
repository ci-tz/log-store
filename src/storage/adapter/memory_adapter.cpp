#include "storage/adapter/memory_adapter.h"

namespace logstore {

MemoryAdapter::MemoryAdapter(int32_t num, int32_t capacity)
    : Adapter(num, capacity), disk_manager_memory_(num * capacity) {}

uint64_t MemoryAdapter::WriteBlock(const char *buf, pba_t pba) {
  disk_manager_memory_.WritePage(pba, buf);
  return 0;
}

uint64_t MemoryAdapter::ReadBlock(char *buf, pba_t pba) {
  disk_manager_memory_.ReadPage(pba, buf);
  return 0;
}

void MemoryAdapter::EraseSegment(seg_id_t id) { UNUSED(id); }

}  // namespace logstore