#pragma once

#include <memory>
#include <unordered_map>

#include "common/config.h"

namespace logstore {

class FIFO {
 public:
  FIFO();
  /**
   * @brief 用于记录LBA的写入时间
   * @param lba 新写入的lba
   * @param avg_lifespan 最热组回收segment的平均寿命，调整FIFO大小的阈值
   */
  void Update(lba_t lba, double avg_lifespan);

  /**
   * @brief 查询LBA的lifespan，如果LBA不存在，则返回无限大的lifespan
   */
  int32_t Query(lba_t lba);

 private:
  int32_t GetQueueSize() const;
  void Shrink();

  int32_t head_ = 0;
  int32_t tail_ = 0;
  std::unique_ptr<int32_t[]> array_;
  std::unordered_map<lba_t, int32_t> lba2pos_;
  const int32_t capacity_ = 128 * 1024 * 1024;
};

};  // namespace logstore