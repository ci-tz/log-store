#include "controller/controller.h"
#include <map>
#include "index/indexmap_factory.h"
#include "select/select_segment_factory.h"
#include "storage/adapter/adapter_factory.h"
#include "storage/adapter/memory_adapter.h"

namespace logstore {

uint64_t Controller::global_timestamp_ = 0;

Controller::Controller(int32_t segment_num, int32_t segment_capacity, double op_ratio,
                       double gc_ratio, const std::string &index_type,
                       const std::string &select_type, const std::string &adapter_type)
    : segment_num_(segment_num),
      segment_capacity_(segment_capacity),
      total_block_num_(segment_num * segment_capacity),
      op_ratio_(op_ratio),
      gc_ratio_(gc_ratio) {
  l2p_map_ = IndexMapFactory::CreateIndexMap(index_type, total_block_num_ * (1 - op_ratio_));
  select_ = SelectSegmentFactory::CreateSelectSegment(select_type);
  adapter_ = AdapterFactory::CreateAdapter(adapter_type, segment_num, segment_capacity);
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
  // Update l2p_map_
  UpdateL2P(lba, pba);
  // Update segment_manager_
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
  segment_manager_->DoGCLeftWork(segment_id);
  invalid_block_num_ -= segment->GetInvalidBlockCount();
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
  // Allocate a new block
  pba = segment_manager_->AllocateFreeBlock();
  LOGSTORE_ASSERT(pba != INVALID_PBA, "Allocate block failed");
  // Write data to the new block
  seg_id_t segment_id = segment_manager_->GetSegmentId(pba);
  off64_t offset = segment_manager_->GetOffset(pba);
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

double Controller::GetFreeSegmentRatio() const { return segment_manager_->GetFreeSegmentRatio(); }

double Controller::GetInvalidBlockRatio() const {
  return static_cast<double>(invalid_block_num_) / total_block_num_;
}

double Controller::GetGcRatio() const { return gc_ratio_; }

}  // namespace logstore