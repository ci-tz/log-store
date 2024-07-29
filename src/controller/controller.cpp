#include "controller/controller.h"
#include <map>
#include "index/array_indexmap.h"
#include "storage/adapter/memory_adapter.h"

namespace logstore {

Controller::Controller(uint32_t segment_num, uint32_t segment_capacity) {
  // TODO: Remove concrete classes
  l2p_map_ = std::make_shared<ArrayIndexMap>(segment_num * segment_capacity);
  p2l_map_ = std::make_shared<ArrayIndexMap>(segment_num * segment_capacity);
  segment_manager_ = std::make_shared<SegmentManager>(segment_num, segment_capacity);
  adapter_ = std::make_shared<MemoryAdapter>(segment_num, segment_capacity);
}

void Controller::ReadBlock(char *buf, lba_t lba) {
  // Get pbas from l2p_map_
  pba_t pba = l2p_map_->Query(lba);
  if (pba == INVALID_PBA) {
    return;
  }
  LOGSTORE_ASSERT(segment_manager_->IsValid(pba), "Invalid pba");
  segment_id_t segment_id = segment_manager_->PBA2SegmentId(pba);
  off64_t offset = segment_manager_->PBA2SegmentOffset(pba);
  adapter_->ReadBlock(buf, segment_id, offset);
}

void Controller::WriteBlock(const char *buf, lba_t lba) {
  // Check if lba has been written before
  pba_t pba = l2p_map_->Query(lba);
  if (pba != INVALID_PBA) {
    // Mark the old block as invalid
    segment_manager_->MarkBlockInvalid(pba);
  }
  // Allocate a new block
  pba = segment_manager_->AllocateFreeBlock();
  LOGSTORE_ASSERT(pba != INVALID_PBA, "Allocate block failed");
  // Write data to the new block
  segment_id_t segment_id = segment_manager_->PBA2SegmentId(pba);
  off64_t offset = segment_manager_->PBA2SegmentOffset(pba);
  adapter_->WriteBlock(buf, segment_id, offset);
  // Update l2p_map_
  l2p_map_->Update(lba, pba);
  // Update p2l_map_
  p2l_map_->Update(pba, lba);
  // Update segment_manager_
  segment_manager_->MarkBlockValid(pba, lba);
  // Update user_write_cnt_
  user_write_cnt_++;
  // Update timestamp_
  timestamp_++;
}

}  // namespace logstore