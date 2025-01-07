#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "common/config.h"
#include "index/array_indexmap.h"
#include "index/hash_indexmap.h"
#include "index/indexmap.h"

namespace logstore {

class IndexMapFactory {
 public:
  static std::shared_ptr<IndexMap> GetIndexMap(const std::string &type) {
    int32_t max_lba = Config::GetInstance().seg_cap * Config::GetInstance().seg_num;

    if (type == "Array") {
      return std::make_shared<ArrayIndexMap>(max_lba);
    } else if (type == "Hash") {
      return std::make_shared<HashIndexMap>(max_lba);
    } else {
      std::cerr << "Unknown indexmap type: " << type << std::endl;
      return nullptr;
    }
  }
};

}  // namespace logstore