#include "framework/controller.h"
#include <string>
#include "common/macros.h"
#include "gtest/gtest.h"

namespace logstore {

TEST(ControllerTest, DISABLED_ReadWriteTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr double op_ratio = 0.25;
  constexpr lba_t max_lba = segment_num * segment_capacity * (1 - op_ratio);
  Controller *controller =
      new Controller(segment_num, segment_capacity, op_ratio, "Array", "Greedy", "Memory");
  char write_buf[BLOCK_SIZE];
  char read_buf[BLOCK_SIZE];
  // Sequential write
  for (lba_t lba = 0; lba < max_lba; lba++) {
    std::string str = "lba: " + std::to_string(lba);
    memset(write_buf, 0, BLOCK_SIZE);
    memcpy(write_buf, str.c_str(), str.size());
    controller->WriteBlock(write_buf, lba);
  }
  // Sequential read
  for (lba_t lba = 0; lba < max_lba; lba++) {
    controller->ReadBlock(read_buf, lba);
    std::string expected = "lba: " + std::to_string(lba);
    std::string actual(read_buf);
    EXPECT_EQ(expected, actual);
  }
  delete controller;
}

TEST(ControllerTest, DISABLED_GCTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr double op_ratio = 0.25;
  constexpr lba_t max_lba = segment_num * segment_capacity * (1 - op_ratio);
  Controller *controller =
      new Controller(segment_num, segment_capacity, op_ratio, "Array", "Greedy", "Memory");

  char write_buf[BLOCK_SIZE];
  char read_buf[BLOCK_SIZE];
  // Sequential write
  for (lba_t lba = 0; lba < max_lba; lba++) {
    std::string buf = "First Write, lba: " + std::to_string(lba);
    memset(write_buf, 0, BLOCK_SIZE);
    memcpy(write_buf, buf.c_str(), buf.size());
    controller->WriteBlock(write_buf, lba);
  }

  /**
   *     0     1     2     3     4       5     6     7
   *  |     |     |     |     |     |       |     |     |
   *  | 0 1 | 2 3 | 4 5 | 6 7 | 8 9 | 10 11 | F F | F F |
   *  |     |     |     |     |     |       |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), op_ratio);

  // Sequential read
  for (lba_t lba = 0; lba < max_lba; lba++) {
    controller->ReadBlock(read_buf, lba);
    std::string expected = "First Write, lba: " + std::to_string(lba);
    std::string actual(read_buf);
    EXPECT_EQ(expected, actual);
  }

  // Update segment 0
  for (lba_t lba = 0; lba < 2; lba++) {
    std::string buf = "Second Write, lba: " + std::to_string(lba);
    memset(write_buf, 0, BLOCK_SIZE);
    memcpy(write_buf, buf.c_str(), buf.size());
    controller->WriteBlock(write_buf, lba);
  }

  /**
   *     0     1     2     3     4       5     6     7
   *  |     |     |     |     |     |       |     |     |
   *  | I I | 2 3 | 4 5 | 6 7 | 8 9 | 10 11 | 0 1 | F F |
   *  |     |     |     |     |     |       |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 1.0 / segment_num);

  controller->DoGC();
  /**
   *     0     1     2     3     4       5     6     7
   *  |     |     |     |     |     |       |     |     |
   *  | F F | 2 3 | 4 5 | 6 7 | 8 9 | 10 11 | 0 1 | F F |
   *  |     |     |     |     |     |       |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 2.0 / segment_num);

  // Read lba 0-1
  for (lba_t lba = 0; lba < 2; lba++) {
    controller->ReadBlock(read_buf, lba);
    std::string expected = "Second Write, lba: " + std::to_string(lba);
    std::string actual(read_buf);
    EXPECT_EQ(expected, actual);
  }

  // Write 2 * segment_capacity blocks
  for (lba_t lba = 0; lba < 2 * segment_capacity; lba++) {
    std::string buf = "Third Write, lba: " + std::to_string(lba);
    controller->WriteBlock(buf.c_str(), lba);
  }

  /**
   *     0     1     2     3     4       5     6     7
   *  |     |     |     |     |     |       |     |     |
   *  | 2 3 | I I | 4 5 | 6 7 | 8 9 | 10 11 | I I | 0 1 |
   *  |     |     |     |     |     |       |     |     |
   */
  // No free segment
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 0);
  controller->DoGC();
  controller->DoGC();
  /**
   *     0     1     2     3     4       5     6     7
   *  |     |     |     |     |     |       |     |     |
   *  | 2 3 | F F | 4 5 | 6 7 | 8 9 | 10 11 | F F | 0 1 |
   *  |     |     |     |     |     |       |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 2.0 / segment_num);

  // Read lba 0-3
  for (lba_t lba = 0; lba < 2 * segment_capacity; lba++) {
    controller->ReadBlock(read_buf, lba);
    std::string expected = "Third Write, lba: " + std::to_string(lba);
    std::string actual(read_buf);
    EXPECT_EQ(expected, actual);
  }

  delete controller;
}

TEST(ControllerTest, DISABLED_MultiReadWriteTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr double op_ratio = 0.25;
  constexpr lba_t max_lba = segment_num * segment_capacity * (1 - op_ratio);
  Controller *controller =
      new Controller(segment_num, segment_capacity, op_ratio, "Array", "Greedy", "Memory");

  char write_buf[BLOCK_SIZE * segment_capacity * segment_num];
  char read_buf[BLOCK_SIZE * segment_capacity * segment_num];
  // write slba:0, len:4
  for (size_t i = 0; i < 4; i++) {
    std::string str = "(0, 4) lba: " + std::to_string(i);
    memset(write_buf + i * BLOCK_SIZE, 0, BLOCK_SIZE);
    memcpy(write_buf + i * BLOCK_SIZE, str.c_str(), str.size());
  }
  controller->WriteMultiBlock(write_buf, 0, 4);
  /**
   *     0     1     2     3     4     5     6     7
   *  |     |     |     |     |     |     |     |     |
   *  | 0 1 | 2 3 | F F | F F | F F | F F | F F | F F |
   *  |     |     |     |     |     |     |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 6.0 / segment_num);
  controller->ReadMultiBlock(read_buf, 0, 4);
  for (size_t i = 0; i < 4; i++) {
    std::string expected = "(0, 4) lba: " + std::to_string(i);
    std::string actual(read_buf + i * BLOCK_SIZE);
    EXPECT_EQ(expected, actual);
  }

  // write slba:1, len: 2
  for (size_t i = 0; i < 2; i++) {
    std::string str = "(1, 2) lba: " + std::to_string(i + 1);
    memset(write_buf + i * BLOCK_SIZE, 0, BLOCK_SIZE);
    memcpy(write_buf + i * BLOCK_SIZE, str.c_str(), str.size());
  }
  controller->WriteMultiBlock(write_buf, 1, 2);
  /**
   *     0     1     2     3     4     5     6     7
   *  |     |     |     |     |     |     |     |     |
   *  | 0 I | I 3 | 1 2 | F F | F F | F F | F F | F F |
   *  |     |     |     |     |     |     |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 5.0 / segment_num);
  EXPECT_EQ(controller->GetInvalidBlockRatio(), 2.0 / (segment_num * segment_capacity));
  controller->ReadMultiBlock(read_buf, 0, 4);
  for (size_t i = 0; i < 4; i++) {
    std::string expected;
    if (i == 0 || i == 3) {
      expected = "(0, 4) lba: ";
    } else {
      expected = "(1, 2) lba: ";
    }
    expected += std::to_string(i);
    std::string actual(read_buf + i * BLOCK_SIZE);
    EXPECT_EQ(expected, actual);
  }
  controller->DoGC();
  controller->DoGC();
  /**
   *     0     1     2     3     4     5     6     7
   *  |     |     |     |     |     |     |     |     |
   *  | F F | F F | 1 2 | 0 3 | F F | F F | F F | F F |
   *  |     |     |     |     |     |     |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 6.0 / segment_num);
  EXPECT_EQ(controller->GetInvalidBlockRatio(), 0);
  controller->ReadMultiBlock(read_buf, 0, 4);
  for (size_t i = 0; i < 4; i++) {
    std::string expected;
    if (i == 0 || i == 3) {
      expected = "(0, 4) lba: ";
    } else {
      expected = "(1, 2) lba: ";
    }
    expected += std::to_string(i);
    std::string actual(read_buf + i * BLOCK_SIZE);
    EXPECT_EQ(expected, actual);
  }

  // write slba:0, len: max_lba
  for (lba_t lba = 0; lba < max_lba; lba++) {
    std::string str = "(0, max_lba) lba: " + std::to_string(lba);
    memset(write_buf + lba * BLOCK_SIZE, 0, BLOCK_SIZE);
    memcpy(write_buf + lba * BLOCK_SIZE, str.c_str(), str.size());
  }
  controller->WriteMultiBlock(write_buf, 0, max_lba);
  /**
   *     0       1     2     3     4     5     6     7
   *  |     |       |     |     |     |     |     |     |
   *  | 8 9 | 10 11 | I I | I I | 0 1 | 2 3 | 4 5 | 6 7 |
   *  |     |       |     |     |     |     |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 0 / segment_num);
  EXPECT_EQ(controller->GetInvalidBlockRatio(), 4.0 / (segment_num * segment_capacity));
  controller->DoGC();
  controller->DoGC();
  /**
   *     0       1     2     3     4     5     6     7
   *  |     |       |     |     |     |     |     |     |
   *  | 8 9 | 10 11 | F F | F F | 0 1 | 2 3 | 4 5 | 6 7 |
   *  |     |       |     |     |     |     |     |     |
   */
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 2.0 / segment_num);
  EXPECT_EQ(controller->GetInvalidBlockRatio(), 0);
  controller->ReadMultiBlock(read_buf, 0, max_lba);
  for (lba_t lba = 0; lba < max_lba; lba++) {
    std::string expected = "(0, max_lba) lba: " + std::to_string(lba);
    std::string actual(read_buf + lba * BLOCK_SIZE);
    EXPECT_EQ(expected, actual);
  }

  delete controller;
}

}  // namespace logstore