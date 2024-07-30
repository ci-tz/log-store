#include "storage/adapter/adapter.h"
#include "storage/disk/disk_manager_memory.h"

namespace logstore {

class MemoryAdapter : public Adapter {
 public:
  MemoryAdapter(uint32_t segment_num, uint32_t segment_capacity);
  ~MemoryAdapter() override = default;

  void WriteBlock(const char *buf, seg_id_t segment_id, off64_t offset) override;

  void ReadBlock(char *buf, seg_id_t segment_id, off64_t offset) override;

 private:
  DiskManagerMemory disk_manager_memory_;
};

}  // namespace logstore