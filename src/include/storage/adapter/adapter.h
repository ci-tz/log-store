#pragma once

#include <vector>
#include "common/config.h"
#include "common/macros.h"

namespace logstore {

class Adapter {
 public:
  /**
   * @brief Construct a new Adapter object
   * @param num The number of segments
   * @param capacity The number of blocks in a segment
   */
  explicit Adapter(int32_t num, int32_t capacity) : num_(num), capacity_(capacity) {}

  /**
   * @brief Destroy the Adapter object
   */
  virtual ~Adapter() = default;

  /**
   * @brief Write a block to the adapter
   * @param buf The buffer to write
   * @param id The segment id
   * @param offset The offset in the segment
   */
  virtual void WriteBlock(const char *buf, int32_t id, off64_t offset) = 0;

  /**
   * @brief Read a block from the adapter
   * @param buf The buffer to read
   * @param id The segment id
   * @param offset The offset in the segment
   */
  virtual void ReadBlock(char *buf, int32_t id, off64_t offset) = 0;

 protected:
  int32_t num_;       // The number of segments
  int32_t capacity_;  // The number of blocks in a segment
};

}  // namespace logstore