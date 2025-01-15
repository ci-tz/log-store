
#include "placement/dac.h"
#include "common/logger.h"
#include "common/macros.h"

namespace logstore {

int32_t DAC::Classify(lba_t lba, bool is_gc_write) {
  UNUSED(is_gc_write);
  int32_t temprature = lba_temprature_[lba];
  LOG_DEBUG("lba: %d, temprature: %d", lba, temprature);
  return temprature;
}

void DAC::MarkUserAppend(lba_t lba, uint64_t timestamp) {
  UNUSED(timestamp);
  if (lba_temprature_.find(lba) == lba_temprature_.end()) {
    lba_temprature_[lba] = 0;
  } else {
    int32_t temprature = lba_temprature_[lba];
    if (temprature < group_num_ - 1) {
      lba_temprature_[lba] = temprature + 1;
    }
  }
}

void DAC::MarkGcAppend(lba_t lba) {
  LOGSTORE_ASSERT(lba_temprature_.find(lba) != lba_temprature_.end(), "lba not found");
  int32_t temprature = lba_temprature_[lba];
  if (temprature != 0) {
    lba_temprature_[lba] = temprature - 1;
  }
}

void DAC::MarkCollectSegment(std::shared_ptr<Segment> ptr) { UNUSED(ptr); }

};  // namespace logstore