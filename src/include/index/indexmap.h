#pragma once

#include "common/config.h"

#include <memory>

namespace logstore {

/**
 * IndexMap is an abstract class that defines the interface for a mapping from logical block
 * addresses (LBAs) to physical block addresses (PBAs). Thread safety: all methods must be
 * thread-safe.
 */
class IndexMap {
 public:
  virtual pba_t Query(lba_t) = 0;
  virtual void Update(lba_t, pba_t) = 0;

  IndexMap() = default;
  virtual ~IndexMap() = default;
};

/**
 * Factory class for creating IndexMap instances.
 */
class IndexMapFactory {
 public:
  IndexMapFactory() = default;
  virtual std::shared_ptr<IndexMap> Create(int32_t max_size) = 0;
  virtual ~IndexMapFactory() = default;
};

}  // namespace logstore