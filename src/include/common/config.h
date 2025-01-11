#pragma once

#include <cstdint>
#include <string>

namespace logstore {
using seg_id_t = int32_t;
using page_id_t = int32_t;
using lba_t = int32_t;
using pba_t = int32_t;

#define INVALID_LBA (-1)
#define INVALID_PBA (-1)
#define BLOCK_SIZE (4096)                             // 4KB
#define SEGMENT_SIZE (512 * 1024 * 1024)              // 512MB
#define SEGMENT_CAPACITY (SEGMENT_SIZE / BLOCK_SIZE)  // 128K blocks
#define SEGMENT_NUM (16)                              // 16 segments

class Config {
 public:
  static Config &GetInstance() {
    static Config instance;
    return instance;
  }

  int32_t opened_segment_num = 1;
  int32_t seg_cap = 131072;  // Segment Size: 512MB
  int32_t seg_num = 16;      // Total Size: 8GB
  double op = 0.25;
  std::string placement = "NoPlacement";
  std::string adapter = "NoAdapter";
  std::string selection = "Greedy";
  std::string index_map = "Array";

  // For local file system backend
  std::string localAdapterDir = "/tmp/local";
};

}  // namespace logstore