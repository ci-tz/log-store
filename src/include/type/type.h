#pragma once

#include <cstdint>

namespace logstore {

// Logical block address
using lba_t = uint32_t;
// Physical block address
using pba_t = uint32_t;

constexpr lba_t INVALID_LBA = -1;  // 0xFFFFFFFF
constexpr pba_t INVALID_PBA = -1;  // 0xFFFFFFFF

constexpr uint32_t kBlockSize = 4096;                             // 4KB
constexpr uint32_t kSegmentSize = 512 * 1024 * 1024;              // 512MB
constexpr uint32_t kSegmentCapacity = kSegmentSize / kBlockSize;  // 131072

}  // namespace logstore