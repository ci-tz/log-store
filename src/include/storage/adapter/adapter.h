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
   * @return uint64_t Latency in microseconds, only used when simulating
   */
  virtual uint64_t WriteBlock(const char *buf, pba_t pba) = 0;

  /**
   * @brief Read a block from the adapter
   * @param buf The buffer to read
   * @param id The segment id
   * @param offset The offset in the segment
   * @return uint64_t Latency in microseconds, only used when simulating
   */
  virtual uint64_t ReadBlock(char *buf, pba_t pba) = 0;

  /**
   * @brief Erase a segment
   */
  virtual void EraseSegment(seg_id_t seg_id) = 0;

 protected:
  int32_t num_;       // The number of segments
  int32_t capacity_;  // The number of blocks in a segment
};

}  // namespace logstore