#pragma once

#include <cstdint>

namespace logstore {
using seg_id_t = int32_t;
using page_id_t = int32_t;
using lba_t = int32_t;
using pba_t = int32_t;

static constexpr lba_t INVALID_LBA = -1;
static constexpr pba_t INVALID_PBA = -1;
static constexpr uint32_t BLOCK_SIZE = 4096;  // 4KB
}  // namespace logstore