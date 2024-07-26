#pragma once

#include <mutex>
#include <unordered_map>
#include "common/rwlatch.h"
#include "index/indexmap.h"

namespace logstore {

class HashIndexMap : public IndexMap {
 public:
  explicit HashIndexMap(int32_t max_size);
  HashIndexMap() = delete;
  virtual ~HashIndexMap() override = default;

  pba_t Query(lba_t) override;
  void Update(lba_t, pba_t) override;

 private:
  inline void WLatch() { latch_.WLock(); }
  inline void WUnlatch() { latch_.WUnlock(); }
  inline void RLatch() { latch_.RLock(); }
  inline void RUnlatch() { latch_.RUnlock(); }

  int32_t max_size_;
  std::unordered_map<lba_t, pba_t> index_map_;
  ReaderWriterLatch latch_;
};

class HashIndexMapFactory : public IndexMapFactory {
 public:
  HashIndexMapFactory() = default;
  std::shared_ptr<IndexMap> Create(int32_t max_size) override { return std::make_shared<HashIndexMap>(max_size); }
};

}  // namespace logstore