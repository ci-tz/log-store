#pragma once

#include <mutex>
#include <unordered_map>
#include "common/rwlatch.h"
#include "index/indexmap.h"

namespace logstore {

class HashIndexMap : public IndexMap {
 public:
  virtual ~HashIndexMap() override = default;

  pba_t Query(lba_t) override;
  void Update(lba_t, pba_t) override;

 private:
  std::unordered_map<lba_t, pba_t> index_map_;
};

}  // namespace logstore