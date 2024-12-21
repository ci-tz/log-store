#pragma once

#include <memory>
#include "storage/adapter/adapter.h"
#include "storage/adapter/local_adapter.h"
#include "storage/adapter/memory_adapter.h"

namespace logstore {

class AdapterFactory {
 public:
  static std::shared_ptr<Adapter> CreateAdapter(int32_t num, int32_t capacity, std::string type) {
    if (type == "Memory") {
      return std::make_shared<MemoryAdapter>(num, capacity);
    } else if (type == "Local") {
      return std::make_shared<LocalAdapter>(num, capacity);
    } else {
      return nullptr;
    }
  }
};

}  // namespace logstore