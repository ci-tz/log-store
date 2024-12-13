#include "storage/adapter/memory_adapter.h"

namespace logstore {

MemoryAdapter::MemoryAdapter(int32_t num, int32_t capacity)
    : Adapter(num, capacity), disk_manager_memory_(num * capacity) {}

void MemoryAdapter::WriteBlock(const char *buf, int32_t id, off64_t offset) {
  page_id_t page_id = id * segment_capacity_ + offset;
  disk_manager_memory_.WritePage(page_id, buf);
}

void MemoryAdapter::ReadBlock(char *buf, int32_t id, off64_t offset) {
  page_id_t page_id = id * segment_capacity_ + offset;
  disk_manager_memory_.ReadPage(page_id, buf);
}

}  // namespace logstore