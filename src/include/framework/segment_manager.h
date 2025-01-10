#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
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

  SegmentManager(int32_t seg_num, int32_t seg_cap, double op);

  virtual ~SegmentManager();

  uint64_t UserAppendBlock(lba_t lba);

  uint64_t UserReadBlock(lba_t lba);

  /**
   * @brief 判断当前系统是否需要GC
   * @return 0: 不需要GC； 1: 需要后台GC；2: 需要强制GC；
   */
  int32_t ShouldGc();

  /**
   * @brief 执行GC操作，选择一个victim segment进行回收
   */
  void DoGc(bool force);

  void PrintSegmentsInfo();

  friend class GcDaemon;

 private:
  /**
   * @brief 根据LBA查找对应的PBA, 若找不到，则返回INVALID_PBA(-1)
   */
  pba_t SearchL2P(lba_t lba) const;

  /**
   * @brief 更新L2P表，将lba映射到pba
   */
  void UpdateL2P(lba_t lba, pba_t pba);

  /**
   * @brief 找到pba所属的segment, 返回nullptr表示找不到
   */
  std::shared_ptr<Segment> GetSegment(pba_t pba);

  /**
   * @brief 从系统中分配一个空闲的segment
   */
  std::shared_ptr<Segment> AllocFreeSegment(int32_t group_id);

  /**
   * @brief 根据确定的GC选择策略，从所有sealed segments中选择一个victim segment
   */
  seg_id_t SelectVictimSegment();

  /**
   * @brief GC时，读取segment中所有的有效数据
   * @param victim 被选择GC的segment id
   */
  void GcReadSegment(seg_id_t victim);

  /**
   * @brief GC时，擦除victim segment中的所有数据块
   */
  void GcEraseSegment(seg_id_t victim);

  /**
   * @brief GC时，将读取出的有效数据写入新的segment中，并更新L2P表
   * @param lba 有效数据的LBA
   * @param old_pba 有效数据的旧PBA
   */
  void GcAppendBlock(lba_t lba, pba_t old_pba);

  int32_t seg_num_ = 0;
  int32_t seg_cap_ = 0;
  double op_ratio_ = 0.0;
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

  std::mutex global_mutex_;
  std::condition_variable cv_;
};

}  // namespace logstore