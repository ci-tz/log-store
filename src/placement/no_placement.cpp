

#include "placement/no_placement.h"
#include "common/macros.h"

namespace logstore {

int32_t NoPlacement::Classify(lba_t lba, bool is_gc_write) {
  UNUSED(lba);
  UNUSED(is_gc_write);
  return 0;
}

void NoPlacement::MarkUserAppend(lba_t lba, uint64_t timestamp) {
  UNUSED(lba);
  UNUSED(timestamp);
}

void NoPlacement::MarkGcAppend(lba_t lba) { UNUSED(lba); }

void NoPlacement::MarkCollectSegment(std::shared_ptr<Segment> ptr) { UNUSED(ptr); };

};  // namespace logstore