#include "index/hash_indexmap.h"
#include "common/macros.h"

namespace logstore {

HashIndexMap::HashIndexMap(int32_t max_size) : max_size_(max_size) {
  index_map_.reserve(max_size);
  // Set pba_t to INVALID_PBA
  for (lba_t lba = 0; lba < max_size; lba++) {
    index_map_[lba] = INVALID_PBA;
  }
}

pba_t HashIndexMap::Query(lba_t lba) {
  if (lba >= max_size_) {
    return INVALID_PBA;
  }
  RLatch();
  auto it = index_map_.find(lba);
  pba_t pba = (it == index_map_.end()) ? INVALID_PBA : it->second;
  RUnlatch();
  return pba;
}

void HashIndexMap::Update(lba_t lba, pba_t pba) {
  WLatch();
  index_map_[lba] = pba;
  WUnlatch();
}

}  // namespace logstore