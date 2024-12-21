#pragma once

#include <cstdint>
#include <string>

namespace logstore {
using seg_id_t = int32_t;
using page_id_t = int32_t;
using lba_t = int32_t;
using pba_t = int32_t;

static constexpr lba_t INVALID_LBA = -1;
static constexpr pba_t INVALID_PBA = -1;
static constexpr uint32_t BLOCK_SIZE = 4096;  // 4KB

class Config {
 public:
  static Config &GetInstance() {
    static Config instance;
    return instance;
  }

  std::string storageAdapter = "Local";

  // For local file system backend
  std::string localAdapterDir = "/tmp/local";
};

}  // namespace logstore