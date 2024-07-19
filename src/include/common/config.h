#pragma once

#include <cstdint>
#include "type/type.h"

namespace logstore {

class Config {
 public:
  // Singleton pattern
  static Config &GetInstance() {
    static Config instance;
    return instance;
  }
  static constexpr uint32_t kBlockSize = 4096;                             // 4KB
  static constexpr uint32_t kSegmentSize = 512 * 1024 * 1024;              // 512MB
  static constexpr uint32_t kSegmentCapacity = kSegmentSize / kBlockSize;  // 131072

 private:
  Config() = default;
  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;
  Config(Config &&) = delete;
  Config &operator=(Config &&) = delete;
};

}  // namespace logstore