#include <string>
#include "common/macros.h"
#include "controller/controller.h"
#include "gtest/gtest.h"

namespace logstore {

TEST(ControllerGCTEST, GC) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr double op_ratio = 0.75;
  constexpr lba_t max_lba = segment_num * segment_capacity * op_ratio;
  // constexpr pba_t max_pba = segment_num * segment_capacity;
  Controller *controller = new Controller(segment_num, segment_capacity);

  // Sequential write
  for (lba_t lba = 0; lba < max_lba; lba++) {
    std::string buf = "First Write, lba: " + std::to_string(lba);
    controller->WriteBlock(buf.c_str(), lba);
  }

  EXPECT_EQ(controller->GetFreeSegmentRatio(), (1 - op_ratio));

  // Sequential read
  for (lba_t lba = 0; lba < max_lba; lba++) {
    char buf[BLOCK_SIZE];
    controller->ReadBlock(buf, lba);
    std::string expected = "First Write, lba: " + std::to_string(lba);
    std::string actual(buf);
    EXPECT_EQ(expected, actual);
  }

  // Update segment 0
  for (lba_t lba = 0; lba < 2; lba++) {
    std::string buf = "Second Write, lba: " + std::to_string(lba);
    controller->WriteBlock(buf.c_str(), lba);
  }
  // Only one segment is free
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 1.0 / segment_num);
  controller->DoGC();
  // One segment is cleaned
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 2.0 / segment_num);

  // Read lba 0/1
  for (lba_t lba = 0; lba < 2; lba++) {
    char buf[BLOCK_SIZE];
    controller->ReadBlock(buf, lba);
    std::string expected = "Second Write, lba: " + std::to_string(lba);
    std::string actual(buf);
    EXPECT_EQ(expected, actual);
  }

  // Write 2 * segment_capacity blocks
  for (lba_t lba = 0; lba < 2 * segment_capacity; lba++) {
    std::string buf = "Third Write, lba: " + std::to_string(lba);
    controller->WriteBlock(buf.c_str(), lba);
  }
  // No free segment
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 0);
  controller->DoGC();
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 1.0 / segment_num);
  controller->DoGC();
  EXPECT_EQ(controller->GetFreeSegmentRatio(), 2.0 / segment_num);

  // Read lba 0-3
  for (lba_t lba = 0; lba < 2 * segment_capacity; lba++) {
    char buf[BLOCK_SIZE];
    controller->ReadBlock(buf, lba);
    std::string expected = "Third Write, lba: " + std::to_string(lba);
    std::string actual(buf);
    EXPECT_EQ(expected, actual);
  }

  delete controller;
}

}  // namespace logstore