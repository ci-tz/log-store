#include <iostream>

#include "framework/segment_manager.h"
#include "placement/sepbit.h"

namespace logstore {

SepBIT::SepBIT() { fifo_ = std::make_unique<FIFO>(); }

int SepBIT::Classify(lba_t lba, bool is_gc_write) {
  if (!is_gc_write) {
    int32_t lifespan = fifo_->Query(lba);
    if (lifespan != INT32_MAX && lifespan < avg_lifespan_) {
      return 0;
    } else {
      return 1;
    }
  } else {
    if (group_id_of_last_collected_segment_ == 0) {
      return 2;
    } else {
      uint64_t last_write = last_write_time_[lba];
      uint64_t age = SegmentManager::write_timestamp - last_write;
      if (age < 4 * avg_lifespan_) {
        return 3;
      } else if (age < 16 * avg_lifespan_) {
        return 4;
      } else {
        return 5;
      }
    }
  }
}

void SepBIT::MarkUserAppend(lba_t lba, uint64_t timestamp) {
  fifo_->Update(lba, avg_lifespan_);
  last_write_time_[lba] = timestamp;
}

void SepBIT::MarkGcAppend(lba_t lba) { UNUSED(lba); }

void SepBIT::MarkCollectSegment(std::shared_ptr<Segment> ptr) {
  static int32_t total_lifespan = 0;
  static int32_t total_cnt = 0;
  if (ptr->GetGroupID() == 0) {
    total_lifespan += ptr->GetAge();
    total_cnt++;
  }

  if (total_cnt == 16) {
    avg_lifespan_ = 1.0 * total_lifespan / total_cnt;
    total_lifespan = 0;
    total_cnt = 0;
    std::cout << "[SepBIT] AvgLifespan: " << avg_lifespan_ << std::endl;
  }
  group_id_of_last_collected_segment_ = ptr->GetGroupID();
}

};  // namespace logstore