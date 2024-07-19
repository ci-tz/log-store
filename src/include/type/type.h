#pragma once

#include <cstdint>

namespace logstore {

// Logical block address
using lba_t = uint32_t;
// Physical block address
using pba_t = uint32_t;

constexpr lba_t INVALID_LBA = -1;  // 0xFFFFFFFF
constexpr pba_t INVALID_PBA = -1;  // 0xFFFFFFFF
}  // namespace logstore