#include "storage/adapter/local_adapter.h"
#include <iostream>
#include "common/config.h"

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

void LocalAdapter::WriteBlock(const char *buf, int32_t id, off64_t offset) {
  // Check if offset is within the segment
  off64_t next_offset = offset_map_[id];
  if (offset < 0 || offset >= capacity_ || offset != next_offset) {
    std::cerr << "Invalid offset " << offset << std::endl;
    return;
  }
  file_map_[id].seekp(offset * BLOCK_SIZE);
  file_map_[id].write(buf, BLOCK_SIZE);
  offset_map_[id]++;
}

void LocalAdapter::ReadBlock(char *buf, int32_t id, off64_t offset) {
  // Check if offset is within the segment
  if (offset < 0 || offset >= capacity_) {
    std::cerr << "Invalid offset " << offset << std::endl;
    return;
  }
  file_map_[id].seekg(offset * BLOCK_SIZE);
  file_map_[id].read(buf, BLOCK_SIZE);
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

  std::cout << "LocalAdapter Create segment " << id << std::endl;
}

void LocalAdapter::DestroySegment(int32_t id) {
  std::string file_name = dir_ + "/segment_" + std::to_string(id);
  file_map_[id].close();
  file_map_.erase(id);
  std::remove(file_name.c_str());
  std::cout << "LocalAdapter Destroy segment " << id << std::endl;
}

}  // namespace logstore