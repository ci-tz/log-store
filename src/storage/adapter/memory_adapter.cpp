#include "storage/adapter/memory_adapter.h"

namespace logstore {

MemoryAdapter::MemoryAdapter(uint32_t segment_num, uint32_t segment_capacity)
    : Adapter(segment_num, segment_capacity),
      disk_manager_memory_(segment_num * segment_capacity) {}

void MemoryAdapter::WriteBlock(const char *buf, seg_id_t segment_id, off64_t offset) {
  page_id_t page_id = segment_id * segment_capacity_ + offset;
  disk_manager_memory_.WritePage(page_id, buf);
}

void MemoryAdapter::ReadBlock(char *buf, seg_id_t segment_id, off64_t offset) {
  page_id_t page_id = segment_id * segment_capacity_ + offset;
  disk_manager_memory_.ReadPage(page_id, buf);
}

}  // namespace logstore