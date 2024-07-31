#pragma once
#include <memory>
#include "index/array_indexmap.h"
#include "index/hash_indexmap.h"
#include "index/indexmap.h"

namespace logstore {

class IndexMapFactory {
 public:
  static std::shared_ptr<IndexMap> CreateIndexMap(const std::string &index_type,
                                                  int32_t capacity) {
    if (index_type == "array") {
      return std::make_shared<ArrayIndexMap>(capacity);
    } else if (index_type == "hash") {
      return std::make_shared<HashIndexMap>(capacity);
    } else {
      return nullptr;
    }
  }
};

}  // namespace logstore