#include "storage/adapter/memory_adapter.h"
#include "common/macros.h"
#include "gtest/gtest.h"

namespace logstore {

TEST(MemoryAdapterTest, DISABLED_ReadWriteTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  Adapter *adapter = new MemoryAdapter(segment_num, segment_capacity);
  char write_buf[BLOCK_SIZE];
  char read_buf[BLOCK_SIZE];
  for (int32_t segment_id = 0; segment_id < segment_num; segment_id++) {
    for (off64_t offset = 0; offset < segment_capacity; offset++) {
      std::string str =
          "segment id: " + std::to_string(segment_id) + ", offset: " + std::to_string(offset);
      memcpy(write_buf, str.c_str(), str.size());
      adapter->WriteBlock(write_buf, segment_id, offset);
    }
  }

  for (int32_t segment_id = 0; segment_id < segment_num; segment_id++) {
    for (off64_t offset = 0; offset < segment_capacity; offset++) {
      std::string str =
          "segment id: " + std::to_string(segment_id) + ", offset: " + std::to_string(offset);
      adapter->ReadBlock(read_buf, segment_id, offset);
      EXPECT_EQ(str, std::string(read_buf));
    }
  }
}

}  // namespace logstore