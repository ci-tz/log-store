#pragma once

#include "index/indexmap.h"
#include "segment/segment.h"
#include "segment/segment_manager.h"
#include "storage/adapter/adapter.h"

#include <memory>

namespace logstore {

class Controller {
 public:
  Controller();
  void WriteBlock(const char *buf, lba_t lba);
  void ReadBlock(char *buf, lba_t lba);

 private:
  uint64_t user_write_cnt_ = 0;
  uint64_t gc_write_cnt_ = 0;
  uint64_t timestamp_ = 0;

  std::shared_ptr<IndexMap> l2p_map_;
  std::shared_ptr<IndexMap> p2l_map_;
  std::shared_ptr<SegmentManager> segment_manager_;
  std::shared_ptr<Adapter> adapter_;
};

}  // namespace logstore