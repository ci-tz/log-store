#pragma once

#include "common/config.h"

#include <memory>

namespace logstore {

/**
 * IndexMap is an abstract class that defines the interface for a mapping from logical block
 * addresses (LBAs) to physical block addresses (PBAs). All methods are thread-safe.
 */
class IndexMap {
 public:
  virtual pba_t Query(lba_t lba) = 0;
  virtual void Update(lba_t lba, pba_t pba) = 0;

  IndexMap() = default;
  virtual ~IndexMap() = default;
};

}  // namespace logstore