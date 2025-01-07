#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "common/config.h"
#include "storage/adapter/adapter.h"
#include "storage/adapter/local_adapter.h"
#include "storage/adapter/memory_adapter.h"

namespace logstore {

class AdapterFactory {
 public:
  static std::shared_ptr<Adapter> GetAdapter(std::string type) {
    int32_t num = Config::GetInstance().seg_num;
    int32_t capacity = Config::GetInstance().seg_cap;
    if (type == "Memory") {
      return std::make_shared<MemoryAdapter>(num, capacity);
    } else if (type == "Local") {
      return std::make_shared<LocalAdapter>(num, capacity);
    } else {
      std::cerr << "Unknown adapter type: " << type << std::endl;
      return nullptr;
    }
  }
};

}  // namespace logstore