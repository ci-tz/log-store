#include "index/hash_indexmap.h"
#include "common/macros.h"

namespace logstore {

pba_t HashIndexMap::Query(lba_t lba) {
  if (index_map_.find(lba) == index_map_.end()) {
    return INVALID_PBA;
  }
  return index_map_[lba];
}

void HashIndexMap::Update(lba_t lba, pba_t pba) { index_map_[lba] = pba; }

}  // namespace logstore