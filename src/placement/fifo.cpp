#include "placement/fifo.h"
#include <cassert>

namespace logstore {

FIFO::FIFO() { array_ = std::make_unique<lba_t[]>(max_capacity_); }

void FIFO::Update(lba_t lba, int64_t avg_lifespan) {
  array_[tail_] = lba;
  lba2pos_[lba] = tail_;
  tail_ = (tail_ + 1) % max_capacity_;

  if (GetQueueSize() > avg_lifespan) {
    Shrink();
  }

  if (GetQueueSize() > avg_lifespan) {
    Shrink();
  }
}

int64_t FIFO::Query(lba_t lba) {
  auto it = lba2pos_.find(lba);
  if (it == lba2pos_.end()) {
    return INT32_MAX;
  }
  int32_t pos = it->second;
  assert(array_[pos] == lba);

  int64_t lifespan;
  if (pos < tail_) {
    lifespan = tail_ - pos;
  } else {
    lifespan = max_capacity_ - (pos - tail_);
  }
  return lifespan;
}

void FIFO::Shrink() {
  lba_t old_lba = array_[head_];
  if (lba2pos_[old_lba] == head_) {
    lba2pos_.erase(old_lba);
  }
  head_ = (head_ + 1) % max_capacity_;
}

int32_t FIFO::GetQueueSize() const { return (tail_ + max_capacity_ - head_) % max_capacity_; }

};  // namespace logstore