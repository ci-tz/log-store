#include "controller/controller.h"
#include <map>
#include "index/array_indexmap.h"
#include "storage/adapter/memory_adapter.h"

namespace logstore {

uint64_t Controller::global_timestamp_ = 0;

Controller::Controller(uint32_t segment_num, uint32_t segment_capacity)
    : segment_num_(segment_num), segment_capacity_(segment_capacity) {
  // TODO: Remove concrete classes
  l2p_map_ = std::make_shared<ArrayIndexMap>(segment_num_ * segment_capacity_);
  segment_manager_ = std::make_shared<SegmentManager>(segment_num_, segment_capacity_);
  adapter_ = std::make_shared<MemoryAdapter>(segment_num_, segment_capacity_);
}

pba_t Controller::SearchL2P(lba_t lba) { return l2p_map_->Query(lba); }

void Controller::UpdateL2P(lba_t lba, pba_t pba) { l2p_map_->Update(lba, pba); }

int32_t Controller::ReadSegmentValidBlocks(segment_id_t segment_id, char *data_buf,
                                           lba_t *lba_buf) {
  int32_t valid_block_num = 0;
  Segment *segment = segment_manager_->GetSegment(segment_id);
  off64_t capacity = segment->GetCapacity();
  for (off64_t offset = 0; offset < capacity; offset++) {
    if (segment->IsValid(offset)) {
      adapter_->ReadBlock(data_buf + valid_block_num * BLOCK_SIZE, segment_id, offset);
      lba_buf[valid_block_num] = segment->GetLBA(offset);
      valid_block_num++;
    }
  }
  return valid_block_num;
}

void Controller::WriteBlockGC(const char *buf, lba_t lba) {
  // Allocate a new block
  pba_t pba = segment_manager_->AllocateFreeBlock();
  LOGSTORE_ASSERT(pba != INVALID_PBA, "Allocate block failed");
  // Write data to the new block
  segment_id_t segment_id = segment_manager_->PBA2SegmentId(pba);
  off64_t offset = segment_manager_->PBA2SegmentOffset(pba);
  adapter_->WriteBlock(buf, segment_id, offset);
  // Update l2p_map_
  UpdateL2P(lba, pba);
  // Update segment_manager_
  segment_manager_->MarkBlockValid(pba, lba);
  gc_write_cnt_++;
}

void Controller::DoGC() {
  Segment *victim = segment_manager_->SelectVictimSegment();
  LOGSTORE_ASSERT(victim != nullptr, "Select victim segment failed");
  segment_id_t segment_id = victim->GetSegmentId();

  char *data_buf = new char[BLOCK_SIZE * segment_capacity_];
  lba_t *lba_buf = new lba_t[segment_capacity_];
  int32_t valid_num = ReadSegmentValidBlocks(segment_id, data_buf, lba_buf);
  LOGSTORE_ASSERT(valid_num >= 0, "Read segment valid blocks failed");
  for (int i = 0; i < valid_num; i++) {
    char *buf = data_buf + i * BLOCK_SIZE;
    lba_t lba = lba_buf[i];
    WriteBlockGC(buf, lba);
  }
  segment_manager_->DoGCLeftWork(victim);
}

void Controller::ReadBlock(char *buf, lba_t lba) {
  // Get pbas from l2p_map_
  pba_t pba = SearchL2P(lba);
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
  pba_t pba = SearchL2P(lba);
  if (pba != INVALID_PBA) {
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
  UpdateL2P(lba, pba);
  // Update segment_manager_
  segment_manager_->MarkBlockValid(pba, lba);
  // Update user_write_cnt_
  user_write_cnt_++;
  // Update timestamp_
  global_timestamp_++;
}

}  // namespace logstore