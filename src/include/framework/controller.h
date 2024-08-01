#pragma once

#include "common/channel.h"
#include "common/macros.h"
#include "framework/segment.h"
#include "framework/segment_manager.h"
#include "index/indexmap.h"
#include "storage/adapter/adapter.h"

#include <memory>

namespace logstore {

class Controller {
 public:
  Controller(int32_t segment_num, int32_t segment_capacity, double op_ratio, double gc_ratio,
             const std::string &index_type, const std::string &select_type,
             const std::string &adapter_type);
  virtual ~Controller();

  DISALLOW_COPY_AND_MOVE(Controller);

  // Block I/O
  void WriteMultiBlock(const char *buf, lba_t slba, size_t len);
  void ReadMultiBlock(char *buf, lba_t slba, size_t len);

  void WriteBlock(const char *buf, lba_t lba);
  void ReadBlock(char *buf, lba_t lba);

  // GC
  void DoGC();
  double GetFreeSegmentRatio() const;
  double GetInvalidBlockRatio() const;
  size_t ReadValidBlocks(Segment *segment, char *data_buf, lba_t *lba_buf);
  void WriteBlockGC(const char *buf, lba_t lba);
  double GetGcRatio() const;

  // L2P
  pba_t SearchL2P(lba_t lba);
  void UpdateL2P(lba_t lba, pba_t pba);

 private:
  int32_t segment_num_;
  int32_t segment_capacity_;
  int32_t total_block_num_ = 0;
  int32_t invalid_block_num_ = 0;

  double op_ratio_;
  double gc_ratio_;

  uint64_t user_write_cnt_ = 0;
  uint64_t gc_write_cnt_ = 0;

  char *data_buf_;
  lba_t *lba_buf_;

  static uint64_t global_timestamp_;

  std::shared_ptr<IndexMap> l2p_map_;
  std::shared_ptr<SelectSegment> select_;
  std::shared_ptr<Adapter> adapter_;
  std::shared_ptr<SegmentManager> segment_manager_;
};

}  // namespace logstore