#include "storage/adapter/no_adapter.h"
#include "common/macros.h"

namespace logstore {

NoAdapter::NoAdapter(int32_t num, int32_t capacity) : Adapter(num, capacity) {}

uint64_t NoAdapter::WriteBlock(const char *buf, pba_t pba) {
  UNUSED(buf);
  UNUSED(pba);
  return 0;
}

uint64_t NoAdapter::ReadBlock(char *buf, pba_t pba) {
  UNUSED(buf);
  UNUSED(pba);
  return 0;
}

void NoAdapter::EraseSegment(seg_id_t id) { UNUSED(id); }

}  // namespace logstore