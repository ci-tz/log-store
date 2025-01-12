#include "index/array_indexmap.h"
#include "common/macros.h"

namespace logstore {

ArrayIndexMap::ArrayIndexMap(int32_t max_size) : max_size_(max_size) {
  index_map_ = std::make_unique<pba_t[]>(max_size);
  for (int32_t i = 0; i < max_size; i++) {
    index_map_[i] = INVALID_PBA;
  }
}

pba_t ArrayIndexMap::Query(lba_t lba) {
  int32_t index = static_cast<int32_t>(lba);
  LOGSTORE_ASSERT(index < max_size_, "LBA out of range");
  pba_t pba = index_map_[index];
  return pba;
}

void ArrayIndexMap::Update(lba_t lba, pba_t pba) {
  int32_t index = static_cast<int32_t>(lba);
  LOGSTORE_ASSERT(index < max_size_, "LBA out of range");
  index_map_[index] = pba;
}

}  // namespace logstore