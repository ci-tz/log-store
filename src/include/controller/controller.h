#pragma once

#include "index/indexmap.h"
#include "segment/segment.h"
#include "segment/segment_manager.h"
#include "storage/adapter/adapter.h"

#include <memory>

namespace logstore {

class Controller {
 public:
  Controller(uint32_t segment_num, uint32_t segment_capacity);
  virtual ~Controller() = default;

  // Block I/O
  void WriteBlock(const char *buf, lba_t lba);
  void ReadBlock(char *buf, lba_t lba);

 private:
  uint64_t user_write_cnt_ = 0;
  uint64_t timestamp_ = 0;
  // uint64_t gc_write_cnt_ = 0;
  // uint32_t segment_num_;
  // uint32_t segment_capacity_;

  std::shared_ptr<IndexMap> l2p_map_;
  std::shared_ptr<IndexMap> p2l_map_;
  std::shared_ptr<SegmentManager> segment_manager_;
  std::shared_ptr<Adapter> adapter_;
};

}  // namespace logstore