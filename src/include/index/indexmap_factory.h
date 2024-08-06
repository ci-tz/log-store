#pragma once
#include <memory>
#include "index/array_indexmap.h"
#include "index/hash_indexmap.h"
#include "index/indexmap.h"

namespace logstore {

class IndexMapFactory {
 public:
  static std::shared_ptr<IndexMap> CreateIndexMap(int32_t max_lba, const std::string &type) {
    if (type == "Array") {
      return std::make_shared<ArrayIndexMap>(max_lba);
    } else if (type == "Hash") {
      return std::make_shared<HashIndexMap>(max_lba);
    } else {
      return nullptr;
    }
  }
};

}  // namespace logstore