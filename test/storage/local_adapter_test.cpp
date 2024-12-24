#include "storage/adapter/local_adapter.h"
#include <filesystem>
#include <iostream>
#include "common/macros.h"
#include "gtest/gtest.h"
#include "storage/adapter/adapter_factory.h"

namespace fs = std::filesystem;

namespace logstore {
TEST(LocalAdapterTest, ReadWriteTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 4;
  std::shared_ptr<Adapter> adapter =
      AdapterFactory::CreateAdapter(segment_num, segment_capacity, "Local");

  if (adapter == nullptr) {
    FAIL() << "Adapter creation failed";
  }

  std::string file_dir = Config::GetInstance().localAdapterDir;
  // if file_dir not exist, create it
  if (!fs::exists(file_dir)) {
    fs::create_directory(file_dir);
  }

  char write_buf[BLOCK_SIZE];
  char read_buf[BLOCK_SIZE];
  int cnt = 1;

  std::cout << "[[[ Write Data into Memory Adapter ]]]" << std::endl;
  for (int32_t segment_id = 0; segment_id < segment_num; segment_id++) {
    for (off64_t offset = 0; offset < segment_capacity; offset++) {
      std::string str =
          "segment id: " + std::to_string(segment_id) + ", offset: " + std::to_string(offset);
      memcpy(write_buf, str.c_str(), str.size());
      std::cout << "Line" << cnt++ << ": " << "WRITE: " << write_buf << std::endl;
      adapter->WriteBlock(write_buf, segment_id, offset);
    }
  }

  cnt = 1;
  std::cout << "[[[ Read Data from Memory Adapter ]]]" << std::endl;
  for (int32_t segment_id = 0; segment_id < segment_num; segment_id++) {
    for (off64_t offset = 0; offset < segment_capacity; offset++) {
      std::string str =
          "segment id: " + std::to_string(segment_id) + ", offset: " + std::to_string(offset);
      adapter->ReadBlock(read_buf, segment_id, offset);
      std::cout << "Line" << cnt++ << ": " << "READ: " << read_buf << std::endl;
      EXPECT_EQ(str, std::string(read_buf));
    }
  }

  // Remove the file directory
  fs::remove_all(file_dir);
}

}  // namespace logstore