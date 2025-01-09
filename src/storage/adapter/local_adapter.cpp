#include <iostream>

#include "common/config.h"
#include "storage/adapter/local_adapter.h"

namespace logstore {

LocalAdapter::LocalAdapter(int32_t num, int32_t capacity) : Adapter(num, capacity) {
  for (int i = 0; i < num; i++) {
    CreateSegment(i);
  }
}

LocalAdapter::~LocalAdapter() {
  for (int i = 0; i < num_; i++) {
    DestroySegment(i);
  }
}

uint64_t LocalAdapter::WriteBlock(const char *buf, pba_t pba) {
  seg_id_t id = GetSegmentId(pba);
  off64_t offset = GetOffset(pba);
  off64_t next_offset = offset_map_[id];
  if (offset < 0 || offset >= capacity_ || offset != next_offset) {
    std::cerr << "Invalid offset " << offset << std::endl;
    exit(-1);
  }
  file_map_[id].seekp(offset * BLOCK_SIZE);
  file_map_[id].write(buf, BLOCK_SIZE);
  offset_map_[id]++;
  return 0;
}

uint64_t LocalAdapter::ReadBlock(char *buf, pba_t pba) {
  // Check if offset is within the segment
  seg_id_t id = GetSegmentId(pba);
  off64_t offset = GetOffset(pba);
  if (offset < 0 || offset >= capacity_) {
    std::cerr << "Invalid offset " << offset << std::endl;
    return 0;
  }
  file_map_[id].seekg(offset * BLOCK_SIZE);
  file_map_[id].read(buf, BLOCK_SIZE);
  return 0;
}

void LocalAdapter::CreateSegment(int32_t id) {
  std::string file_name = dir_ + "/segment_" + std::to_string(id);
  std::fstream file(file_name, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
  file_map_[id] = std::move(file);
  offset_map_[id] = 0;
  // Extend the file to the capacity
  file_map_[id].seekp(capacity_ * BLOCK_SIZE - 1);
  // Write a byte to the end of the file
  file_map_[id].write("", 1);
  // Reset the file pointer
  file_map_[id].seekp(0);
}

void LocalAdapter::DestroySegment(int32_t id) {
  std::string file_name = dir_ + "/segment_" + std::to_string(id);
  file_map_[id].close();
  file_map_.erase(id);
  std::remove(file_name.c_str());
  std::cout << "LocalAdapter Destroy segment " << id << std::endl;
}

seg_id_t LocalAdapter::GetSegmentId(pba_t pba) { return pba / capacity_; }

off64_t LocalAdapter::GetOffset(pba_t pba) { return pba % capacity_; }

}  // namespace logstore