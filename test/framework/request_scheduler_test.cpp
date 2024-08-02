#include "framework/request_scheduler.h"
#include "framework/controller.h"
#include "gtest/gtest.h"

namespace logstore {

TEST(RequestSchedulerTest, DISABLED_SeqWriteTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr double op_ratio = 0.25;
  constexpr lba_t max_lba = segment_num * segment_capacity * (1 - op_ratio);
  Controller *controller =
      new Controller(segment_num, segment_capacity, op_ratio, "Array", "Greedy", "Memory");

  RequestScheduler *scheduler = new RequestScheduler(controller);
  /**
   *     0     1     2     3     4     5     6     7
   *  |     |     |     |     |     |     |     |     |
   *  | F F | F F | F F | F F | F F | F F | F F | F F |
   *  |     |     |     |     |     |     |     |     |
   */

  /**
   * write request: (0,2)/(2,2)/(4,2)/(6,2)/(8,2)/(10,2)
   */
  char write_buf[BLOCK_SIZE * segment_capacity];
  for (lba_t slba = 0; slba < max_lba; slba += segment_capacity) {
    std::string buf = "Seq Write, slba: " + std::to_string(slba);
    memset(write_buf, 0, BLOCK_SIZE * segment_capacity);
    memcpy(write_buf, buf.c_str(), buf.size());
    auto promise = scheduler->CreatePromise();
    auto future = promise.get_future();
    scheduler->Schedule({true, write_buf, slba, segment_capacity, std::move(promise)});
    EXPECT_EQ(future.get(), true);
  }
  /**
   *     0     1     2     3     4      5      6     7
   *  |     |     |     |     |     |       |     |     |
   *  | 0 1 | 2 3 | 4 5 | 6 7 | 8 9 | 10 11 | F F | F F |
   *  |     |     |     |     |     |       |     |     |
   */

  char read_buf[BLOCK_SIZE * segment_capacity];
  for (lba_t slba = max_lba - segment_capacity; slba >= 0; slba -= segment_capacity) {
    auto promise = scheduler->CreatePromise();
    auto future = promise.get_future();
    scheduler->Schedule({false, read_buf, slba, segment_capacity, std::move(promise)});
    EXPECT_EQ(future.get(), true);
    std::string expected = "Seq Write, slba: " + std::to_string(slba);
    std::string actual(read_buf);
    EXPECT_EQ(expected, actual);
  }

  delete scheduler;
  delete controller;
}

TEST(RequestSchedulerTest, GCTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr double op_ratio = 0.25;
  constexpr lba_t max_lba = segment_num * segment_capacity * (1 - op_ratio);
  Controller *controller =
      new Controller(segment_num, segment_capacity, op_ratio, "Array", "Greedy", "Memory");

  RequestScheduler *scheduler = new RequestScheduler(controller);
  /**
   *     0     1     2     3     4     5     6     7
   *  |     |     |     |     |     |     |     |     |
   *  | F F | F F | F F | F F | F F | F F | F F | F F |
   *  |     |     |     |     |     |     |     |     |
   */

  /**
   * write request: (0,2)/(2,2)/(4,2)/(6,2)/(8,2)/(10,2)
   */
  char write_buf[BLOCK_SIZE * segment_capacity];
  for (lba_t slba = 0; slba < max_lba; slba += segment_capacity) {
    std::string buf = "First Write, slba: " + std::to_string(slba);
    memset(write_buf, 0, BLOCK_SIZE * segment_capacity);
    memcpy(write_buf, buf.c_str(), buf.size());
    auto promise = scheduler->CreatePromise();
    auto future = promise.get_future();
    scheduler->Schedule({true, write_buf, slba, segment_capacity, std::move(promise)});
    EXPECT_EQ(future.get(), true);
  }
  /**
   *     0     1     2     3     4      5      6     7
   *  |     |     |     |     |     |       |     |     |
   *  | 0 1 | 2 3 | 4 5 | 6 7 | 8 9 | 10 11 | F F | F F |
   *  |     |     |     |     |     |       |     |     |
   */
  char read_buf[BLOCK_SIZE * segment_capacity];
  for (lba_t slba = max_lba - segment_capacity; slba >= 0; slba -= segment_capacity) {
    auto promise = scheduler->CreatePromise();
    auto future = promise.get_future();
    scheduler->Schedule({false, read_buf, slba, segment_capacity, std::move(promise)});
    EXPECT_EQ(future.get(), true);
    std::string expected = "First Write, slba: " + std::to_string(slba);
    std::string actual(read_buf);
    EXPECT_EQ(expected, actual);
  }

  for (lba_t slba = max_lba - segment_capacity; slba >= 0; slba -= segment_capacity) {
    std::string buf = "Second Write, slba: " + std::to_string(slba);
    memset(write_buf, 0, BLOCK_SIZE * segment_capacity);
    memcpy(write_buf, buf.c_str(), buf.size());
    auto promise = scheduler->CreatePromise();
    auto future = promise.get_future();
    scheduler->Schedule({true, write_buf, slba, segment_capacity, std::move(promise)});
    EXPECT_EQ(future.get(), true);
  }

  for (lba_t slba = max_lba - segment_capacity; slba >= 0; slba -= segment_capacity) {
    auto promise = scheduler->CreatePromise();
    auto future = promise.get_future();
    scheduler->Schedule({false, read_buf, slba, segment_capacity, std::move(promise)});
    EXPECT_EQ(future.get(), true);
    std::string expected = "Second Write, slba: " + std::to_string(slba);
    std::string actual(read_buf);
    EXPECT_EQ(expected, actual);
  }

  delete scheduler;
  delete controller;
}

}  // namespace logstore