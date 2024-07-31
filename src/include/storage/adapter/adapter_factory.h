#pragma once

#include <memory>
#include "storage/adapter/adapter.h"
#include "storage/adapter/memory_adapter.h"

namespace logstore {

class AdapterFactory {
 public:
  static std::shared_ptr<Adapter> CreateAdapter(const std::string &adapter_type,
                                                uint32_t segment_num, uint32_t segment_capacity) {
    if (adapter_type == "Memory") {
      return std::make_shared<MemoryAdapter>(segment_num, segment_capacity);
    } else {
      return nullptr;
    }
  }
};

}  // namespace logstore