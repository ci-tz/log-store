#include "placement/fifo.h"

namespace logstore {

FIFO::FIFO() { array_ = std::make_unique<int32_t[]>(capacity_); }

void FIFO::Update(lba_t lba, double avg_lifespan) {
  array_[tail_] = lba;
  lba2pos_[lba] = tail_;
  tail_ = (tail_ + 1) % capacity_;

  int32_t threshold = static_cast<int32_t>(avg_lifespan);
  if (GetQueueSize() > threshold) {
    Shrink();
  }

  if (GetQueueSize() > threshold) {
    Shrink();
  }
}

int32_t FIFO::Query(lba_t lba) {
  auto it = lba2pos_.find(lba);
  if (it == lba2pos_.end()) {
    return INT32_MAX;
  }
  int32_t pos = it->second;
  int32_t lifespan = 0;
  if (pos < tail_) {
    lifespan = tail_ - pos;
  } else {
    lifespan = capacity_ - (pos - tail_);
  }
  return lifespan;
}

void FIFO::Shrink() {
  lba_t old_lba = array_[head_];
  if (lba2pos_[old_lba] == head_) {
    lba2pos_.erase(old_lba);
  }
  head_ = (head_ + 1) % capacity_;
}

int32_t FIFO::GetQueueSize() const { return (tail_ + capacity_ - head_) % capacity_; }

};  // namespace logstore