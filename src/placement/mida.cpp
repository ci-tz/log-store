
#include "placement/mida.h"
#include "common/macros.h"

namespace logstore {

int32_t Mida::Classify(lba_t lba, bool is_gc_write) {
  if (!is_gc_write) {
    return 0;
  }
  // GC Write
  int32_t group_id = lba_group_map_[lba];
  if (group_id < group_num_ - 1) {
    return group_id + 1;
  } else {
    return group_id;
  }
}

void Mida::MarkUserAppend(lba_t lba, uint64_t timestamp) {
  UNUSED(timestamp);
  lba_group_map_[lba] = 0;
}

void Mida::MarkGcAppend(lba_t lba) {
  int32_t group_id = lba_group_map_[lba];
  if (group_id < group_num_ - 1) {
    lba_group_map_[lba] = group_id + 1;
  }
}

void Mida::MarkCollectSegment(std::shared_ptr<Segment> ptr) { UNUSED(ptr); }

};  // namespace logstore