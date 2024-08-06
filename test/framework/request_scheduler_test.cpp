#include "framework/request_scheduler.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "common/logger.h"
#include "framework/controller.h"
#include "gtest/gtest.h"
#include "index/indexmap_factory.h"
#include "select/select_segment_factory.h"
#include "storage/adapter/adapter_factory.h"

namespace logstore {

TEST(RequestSchedulerTest, DISABLED_SeqWriteTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr double op_ratio = 0.25;
  constexpr lba_t max_lba = segment_num * segment_capacity * (1 - op_ratio);
  std::shared_ptr<Adapter> adapter =
      AdapterFactory::CreateAdapter(segment_num, segment_capacity, "Memory");
  std::shared_ptr<IndexMap> index = IndexMapFactory::CreateIndexMap(max_lba, "Array");
  std::shared_ptr<SelectSegment> select = SelectSegmentFactory::CreateSelectSegment("Greedy");

  Controller *controller =
      new Controller(segment_num, segment_capacity, op_ratio, index, select, adapter);

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

TEST(RequestSchedulerTest, DISABLED_GCTest) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr double op_ratio = 0.25;
  constexpr lba_t max_lba = segment_num * segment_capacity * (1 - op_ratio);
  std::shared_ptr<Adapter> adapter =
      AdapterFactory::CreateAdapter(segment_num, segment_capacity, "Memory");
  std::shared_ptr<IndexMap> index = IndexMapFactory::CreateIndexMap(max_lba, "Array");
  std::shared_ptr<SelectSegment> select = SelectSegmentFactory::CreateSelectSegment("Greedy");

  Controller *controller =
      new Controller(segment_num, segment_capacity, op_ratio, index, select, adapter);

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

TEST(RequestSchedulerTest, TraceTest) {
  // Split string by delimiter
  class Internal {
   public:
    static void split(const std::string &s, char delim, std::vector<std::string> &elems) {
      std::stringstream ss(s);
      std::string item;
      while (std::getline(ss, item, delim)) {
        elems.push_back(item);
      }
    }
  };

  const char *trace_file = "../../test/trace/trace-0.csv";  // trace file
  constexpr int32_t segment_capacity = 2048;                // Segment Size: 8MB
  constexpr int32_t segment_num = 8;                        // Device Size: 64MB
  constexpr double op_ratio = 0.25;
  constexpr lba_t max_lba = (1 - op_ratio) * segment_num * segment_capacity;
  char *buf = new char[BLOCK_SIZE * 100];

  std::shared_ptr<Adapter> adapter =
      AdapterFactory::CreateAdapter(segment_num, segment_capacity, "Memory");
  std::shared_ptr<IndexMap> index = IndexMapFactory::CreateIndexMap(max_lba, "Array");
  std::shared_ptr<SelectSegment> select = SelectSegmentFactory::CreateSelectSegment("Greedy");
  Controller *controller =
      new Controller(segment_num, segment_capacity, op_ratio, index, select, adapter);
  RequestScheduler *scheduler = new RequestScheduler(controller);

  std::ifstream trace(trace_file);
  if (trace.fail()) {
    std::cerr << "Failed to open trace file: " << trace_file << std::endl;
    return;
  }
  std::string line;
  while (std::getline(trace, line)) {
    std::vector<std::string> elems;
    Internal::split(line, ',', elems);
    if (elems[0] != "W") {
      continue;
    }
    int32_t slba = std::stoll(elems[1]) / BLOCK_SIZE;
    int32_t elba = slba + std::stoll(elems[2]) / BLOCK_SIZE - 1;
    if (slba >= max_lba) {
      slba = slba % max_lba;
      elba = elba % max_lba;
    } else if (elba >= max_lba) {
      elba = max_lba - 1;
    }
    int32_t len = elba - slba + 1;

    std::stringstream ss;
    ss << "slba: " << slba << ", len: " << len;
    memset(buf, 0, BLOCK_SIZE * 100);
    memcpy(buf, ss.str().c_str(), ss.str().size());

    // Write
    auto promise = scheduler->CreatePromise();
    auto future = promise.get_future();
    scheduler->Schedule({true, buf, slba, len, std::move(promise)});
    EXPECT_EQ(future.get(), true);

    // Read back
    memset(buf, 0, BLOCK_SIZE * 100);
    auto promise2 = scheduler->CreatePromise();
    auto future2 = promise2.get_future();
    scheduler->Schedule({false, buf, slba, len, std::move(promise2)});
    EXPECT_EQ(future2.get(), true);
    std::string expected(ss.str());
    std::string actual(buf);
    EXPECT_EQ(expected, actual);
  }

  delete scheduler;
  delete controller;
}

}  // namespace logstore