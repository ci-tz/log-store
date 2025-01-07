
#include "placement/sepgc.h"
#include "common/macros.h"

namespace logstore {

int32_t SepGC::Classify(lba_t lba, bool is_gc_write) {
  UNUSED(lba);
  if (is_gc_write) {
    return 1;
  } else {
    return 0;
  }
}

void SepGC::MarkUserAppend(lba_t lba, uint64_t timestamp) {
  UNUSED(lba);
  UNUSED(timestamp);
};

void SepGC::MarkGcAppend(lba_t lba) { UNUSED(lba); }

void SepGC::MarkCollectSegment(std::shared_ptr<Segment> ptr) { UNUSED(ptr); }

};  // namespace logstore