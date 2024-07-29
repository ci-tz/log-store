#pragma once

#include <memory>
#include <mutex>
#include "common/rwlatch.h"
#include "index/indexmap.h"

namespace logstore {

class ArrayIndexMap : public IndexMap {
 public:
  explicit ArrayIndexMap(size_t max_size);
  ArrayIndexMap() = delete;

  pba_t Query(lba_t) override;
  void Update(lba_t, pba_t) override;

 private:
  inline void WLatch() { latch_.WLock(); }
  inline void WUnlatch() { latch_.WUnlock(); }
  inline void RLatch() { latch_.RLock(); }
  inline void RUnlatch() { latch_.RUnlock(); }

  size_t max_size_;
  std::unique_ptr<pba_t[]> index_map_;  // lba -> pba
  ReaderWriterLatch latch_;
};

}  // namespace logstore