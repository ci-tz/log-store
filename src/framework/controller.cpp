#include "framework/controller.h"
#include <map>
#include "index/indexmap_factory.h"
#include "select/select_segment_factory.h"
#include "storage/adapter/adapter_factory.h"
#include "storage/adapter/memory_adapter.h"

namespace logstore {

uint64_t Controller::global_timestamp_ = 0;

Controller::Controller(int32_t segment_num, int32_t segment_capacity, double op_ratio,
                       std::shared_ptr<IndexMap> index, std::shared_ptr<SelectSegment> select,
                       std::shared_ptr<Adapter> adapter)
    : segment_num_(segment_num),
      segment_capacity_(segment_capacity),
      total_block_num_(segment_num * segment_capacity),
      op_ratio_(op_ratio),
      l2p_map_(index),
      select_(select),
      adapter_(adapter) {
  segment_manager_ = std::make_shared<SegmentManager>(segment_num_, segment_capacity_, select_);
  data_buf_ = new char[BLOCK_SIZE * segment_capacity_];
  lba_buf_ = new lba_t[segment_capacity_];
}

Controller::~Controller() {
  delete[] data_buf_;
  delete[] lba_buf_;
}

pba_t Controller::SearchL2P(lba_t lba) { return l2p_map_->Query(lba); }

void Controller::UpdateL2P(lba_t lba, pba_t pba) { l2p_map_->Update(lba, pba); }

size_t Controller::ReadValidBlocks(Segment *segment, char *data_buf, lba_t *lba_buf) {
  size_t valid_num = 0;
  seg_id_t segment_id = segment->GetSegmentId();
  off64_t capacity = segment->GetCapacity();
  for (off64_t offset = 0; offset < capacity; offset++) {
    if (segment->IsValid(offset)) {
      adapter_->ReadBlock(data_buf + valid_num * BLOCK_SIZE, segment_id, offset);
      lba_buf[valid_num] = segment->GetLBA(offset);
      valid_num++;
    }
  }
  return valid_num;
}

void Controller::WriteBlockGC(const char *buf, lba_t lba) {
  // Allocate a new block
  pba_t pba = segment_manager_->AllocateFreeBlock();
  LOGSTORE_ASSERT(pba != INVALID_PBA, "Allocate block failed");

  // Write data to the new block
  seg_id_t segment_id = segment_manager_->GetSegmentId(pba);
  off64_t offset = segment_manager_->GetOffset(pba);
  adapter_->WriteBlock(buf, segment_id, offset);

  // Update lba/pba mapping
  UpdateL2P(lba, pba);
  segment_manager_->MarkBlockValid(pba, lba);

  gc_write_cnt_++;
}

void Controller::DoGC() {
  seg_id_t segment_id = segment_manager_->SelectVictimSegment();
  Segment *segment = segment_manager_->GetSegment(segment_id);
  size_t valid_num = ReadValidBlocks(segment, data_buf_, lba_buf_);
  for (size_t i = 0; i < valid_num; i++) {
    char *buf = data_buf_ + i * BLOCK_SIZE;
    lba_t lba = lba_buf_[i];
    WriteBlockGC(buf, lba);
  }
  invalid_block_num_ -= segment->GetInvalidBlockCount();
  segment_manager_->DoGCLeftWork(segment_id);
}

void Controller::ReadBlock(char *buf, lba_t lba) {
  // Get pbas from l2p_map_
  pba_t pba = SearchL2P(lba);
  if (pba == INVALID_PBA) {
    return;
  }
  LOGSTORE_ASSERT(segment_manager_->IsValid(pba), "Invalid pba");
  seg_id_t segment_id = segment_manager_->GetSegmentId(pba);
  off64_t offset = segment_manager_->GetOffset(pba);
  adapter_->ReadBlock(buf, segment_id, offset);
}

void Controller::WriteBlock(const char *buf, lba_t lba) {
  // Check if lba has been written before
  pba_t pba = SearchL2P(lba);
  if (pba != INVALID_PBA) {
    segment_manager_->MarkBlockInvalid(pba);
    invalid_block_num_++;
  }

  pba = segment_manager_->AllocateFreeBlock();
  LOGSTORE_ASSERT(pba != INVALID_PBA, "Allocate block failed");

  // Write data to the new block
  seg_id_t segment_id = segment_manager_->GetSegmentId(pba);
  off64_t offset = segment_manager_->GetOffset(pba);
  adapter_->WriteBlock(buf, segment_id, offset);

  // Update lba/pba mapping
  UpdateL2P(lba, pba);
  segment_manager_->MarkBlockValid(pba, lba);

  user_write_cnt_++;
  global_timestamp_++;
}

void Controller::ReadMultiBlock(char *buf, lba_t slba, size_t len) {
  lba_t end_lba = slba + len - 1;
  for (lba_t lba = slba; lba <= end_lba; lba++) {
    ReadBlock(buf, lba);
    buf += BLOCK_SIZE;
  }
}

void Controller::WriteMultiBlock(char *buf, lba_t slba, size_t len) {
  lba_t end_lba = slba + len - 1;
  for (lba_t lba = slba; lba <= end_lba; lba++) {
    WriteBlock(buf, lba);
    buf += BLOCK_SIZE;
  }
}

double Controller::GetFreeSegmentRatio() const { return segment_manager_->GetFreeSegmentRatio(); }

double Controller::GetInvalidBlockRatio() const {
  return static_cast<double>(invalid_block_num_) / total_block_num_;
}

double Controller::GetOpRatio() const { return op_ratio_; }
}  // namespace logstore