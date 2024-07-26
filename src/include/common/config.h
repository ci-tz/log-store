#pragma once

#include <cstdint>

namespace logstore {

static constexpr uint32_t BLOCK_SIZE = 4096;                             // 4KB
static constexpr uint32_t SEGMENT_SIZE = 512 * 1024 * 1024;              // 512MB
static constexpr uint32_t SEGMENT_CAPACITY = SEGMENT_SIZE / BLOCK_SIZE;  // 131072
using segment_id_t = int32_t;
using page_id_t = int32_t;
// Logical block address
using lba_t = uint32_t;
// Physical block address
using pba_t = uint32_t;

static constexpr lba_t INVALID_LBA = -1;  // 0xFFFFFFFF
static constexpr pba_t INVALID_PBA = -1;  // 0xFFFFFFFF
static constexpr int LOGSTORE_BLK_SIZE = 4096;
}  // namespace logstore