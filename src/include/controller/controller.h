#pragma once

#include "common/macros.h"
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

  DISALLOW_COPY_AND_MOVE(Controller);

  // Block I/O
  void WriteBlock(const char *buf, lba_t lba);
  void ReadBlock(char *buf, lba_t lba);
  void DoGC();
  double GetFreeSegmentRatio() const { return segment_manager_->GetFreeSegmentRatio(); }

 private:
  pba_t SearchL2P(lba_t lba);
  void UpdateL2P(lba_t lba, pba_t pba);

  int32_t ReadSegmentValidBlocks(seg_id_t segment_id, char *data_buf, lba_t *lba_buf);
  void WriteBlockGC(const char *buf, lba_t lba);

  uint64_t user_write_cnt_ = 0;
  uint64_t gc_write_cnt_ = 0;
  static uint64_t global_timestamp_;
  uint32_t segment_num_;
  uint32_t segment_capacity_;

  std::shared_ptr<IndexMap> l2p_map_;
  std::shared_ptr<SegmentManager> segment_manager_;
  std::shared_ptr<Adapter> adapter_;
};

}  // namespace logstore