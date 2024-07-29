#include "index/array_indexmap.h"
#include "common/macros.h"

namespace logstore {

ArrayIndexMap::ArrayIndexMap(size_t max_size) : max_size_(max_size) {
  index_map_ = std::make_unique<pba_t[]>(max_size);
  for (size_t i = 0; i < max_size; i++) {
    index_map_[i] = INVALID_PBA;
  }
}

pba_t ArrayIndexMap::Query(lba_t lba) {
  size_t index = static_cast<size_t>(lba);
  LOGSTORE_ASSERT(index < max_size_, "LBA out of range");
  RLatch();
  pba_t pba = index_map_[index];
  RUnlatch();
  return pba;
}

void ArrayIndexMap::Update(lba_t lba, pba_t pba) {
  size_t index = static_cast<size_t>(lba);
  LOGSTORE_ASSERT(index < max_size_, "LBA out of range");
  WLatch();
  index_map_[index] = pba;
  WUnlatch();
}

}  // namespace logstore