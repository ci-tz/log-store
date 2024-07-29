#include <string>
#include "common/macros.h"
#include "controller/controller.h"
#include "gtest/gtest.h"

namespace logstore {

TEST(ControllerReadWriteTest, ReadWriteTest) {
  constexpr uint32_t segment_num = 8;
  constexpr uint32_t segment_capacity = 2;
  constexpr lba_t max_lba = segment_num * segment_capacity;
  Controller controller(segment_num, segment_capacity);
  std::string test_str = "Hello, world!";
  char buf[BLOCK_SIZE];
  // Sequential write
  for (lba_t lba = 0; lba < max_lba; lba++) {
    std::string buf = test_str + "lba: " + std::to_string(lba);
    controller.WriteBlock(buf.c_str(), lba);
  }
  // Sequential read
  for (lba_t lba = 0; lba < max_lba; lba++) {
    controller.ReadBlock(buf, lba);
    std::string expected = test_str + "lba: " + std::to_string(lba);
    std::string actual(buf);
    EXPECT_EQ(expected, actual);
  }
}

}  // namespace logstore