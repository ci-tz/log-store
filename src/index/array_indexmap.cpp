#include "index/array_indexmap.h"

namespace logstore {

ArrayIndexMap::ArrayIndexMap(int32_t max_size) : max_size_(max_size) {
  index_map_ = std::make_unique<pba_t[]>(max_size);
  for (int32_t i = 0; i < max_size; i++) {
    index_map_[i] = INVALID_PBA;
  }
}

pba_t ArrayIndexMap::Query(lba_t lba) {
  // Chek if the lba is within the range of the index map
  if (lba >= max_size_) {
    throw std::runtime_error("LBA out of range");
  }
  RLatch();
  pba_t pba = index_map_[lba];
  RUnlatch();
  return pba;
}

void ArrayIndexMap::Update(lba_t lba, pba_t pba) {
  // Chek if the lba is within the range of the index map
  if (lba >= max_size_) {
    throw std::runtime_error("LBA out of range");
  }
  WLatch();
  index_map_[lba] = pba;
  WUnlatch();
}

}  // namespace logstore