#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "framework/segment.h"
#include "index/indexmap.h"
#include "placement/placement.h"
#include "select/selection.h"
#include "storage/adapter/adapter.h"

namespace logstore {

class SegmentManager {
 public:
  static uint64_t write_timestamp;

  SegmentManager(int32_t seg_num, int32_t seg_cap);
  virtual ~SegmentManager();

  uint64_t UserAppendBlock(lba_t lba);

  uint64_t UserReadBlock(lba_t lba);

  void GcAppendBlock(lba_t lba, pba_t old_pba);

  void DoGCLeftWork(seg_id_t victim_segment_id);

  void PrintSegmentsInfo();

 private:
  pba_t SearchL2P(lba_t lba) const;
  void UpdateL2P(lba_t lba, pba_t pba);

  /**
   * @brief 找到pba所属的segment, 返回nullptr表示找不到
   */
  std::shared_ptr<Segment> GetSegment(pba_t pba);

  /**
   * @brief 从系统中分配一个空闲的segment
   */
  std::shared_ptr<Segment> AllocFreeSegment();

  /**
   * @brief 根据确定的GC选择策略，从所有sealed segments中选择一个victim segment
   */
  seg_id_t SelectVictimSegment();

  /**
   * @brief 对segment进行GC操作，将所有有效数据迁移到新的segment中，并释放旧的segment
   */
  void GcSegment(std::shared_ptr<Segment> seg_ptr);

  int32_t seg_num_ = 0;
  int32_t seg_cap_ = 0;
  int32_t total_blocks_ = 0;
  int32_t total_invalid_blocks_ = 0;

  int64_t total_user_writes_ = 0;
  int64_t total_gc_writes_ = 0;

  std::shared_ptr<IndexMap> l2p_map_;     // LBA -> PBA
  std::shared_ptr<Selection> selection_;  // GC选择策略
  std::shared_ptr<Adapter> adapter_;
  std::shared_ptr<Placement> placement_;

  std::unordered_map<seg_id_t, std::shared_ptr<Segment>> segments_;  // segment_id -> segment
  std::vector<std::shared_ptr<Segment>> opened_segments_;
  std::unordered_set<std::shared_ptr<Segment>> sealed_segments_;
  std::unordered_set<std::shared_ptr<Segment>> free_segments_;
};

}  // namespace logstore