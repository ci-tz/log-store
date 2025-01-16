#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "framework/phy_segment.h"
#include "framework/segment.h"
#include "index/indexmap.h"
#include "placement/placement.h"
#include "probe/gc_lifespan.h"
#include "select/selection.h"
#include "storage/adapter/adapter.h"

namespace logstore {

using SegmentSet = std::unordered_set<std::shared_ptr<Segment>>;

using SegmentVector = std::vector<std::shared_ptr<Segment>>;

class SegmentManager {
 public:
  static uint64_t write_timestamp;

  SegmentManager();

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
   * @return 是否成功执行GC操作
   */
  bool DoGc(bool force);

  void PrintSegmentsInfo() const;

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
  std::shared_ptr<Segment> AllocFreeSegment(class_id_t class_id);

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

  // Physical segments
  std::unordered_map<seg_id_t, std::shared_ptr<PhySegment>> phy_segments_;
  std::unordered_set<std::shared_ptr<PhySegment>> free_phy_segments_;

  // Logical segments
  std::unordered_map<level_id_t, int32_t> write_pointers_;  // level id --> Which segment to write next
  std::unordered_map<level_id_t, SegmentVector> opened_segments_;
  std::unordered_map<class_id_t, SegmentSet> sealed_segments_;
  std::unordered_map<class_id_t, SegmentSet> free_segments_;
};

}  // namespace logstore