#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "common/config.h"
#include "common/zipf_distribution.h"
#include "framework/gc_daemon.h"
#include "framework/segment_manager.h"
#include "gtest/gtest.h"
#include "select/selection_factory.h"

namespace logstore {

TEST(SegmentManagerTest, SeqWrite) {
  Config &config = Config::GetInstance();
  config.placement = "SepGC";
  config.adapter = "NoAdapter";
  config.selection = "Greedy";
  config.opened_segment_num = 2;

  constexpr int32_t seg_num = 64;
  constexpr int32_t seg_cap = 128;
  constexpr double op = 0.25;
  constexpr int32_t max_lba = seg_num * (1 - op) * seg_cap;
  constexpr int32_t write_cnt = max_lba * 4;

  std::shared_ptr<SegmentManager> manager = std::make_shared<SegmentManager>(seg_num, seg_cap, op);

  std::shared_ptr<GcDaemon> gc_daemon = std::make_shared<GcDaemon>(manager);

  // write the sequence
  std::cout << "----------Warm Up----------" << std::endl;
  for (lba_t lba = 0; lba < max_lba; lba++) {
    manager->UserAppendBlock(lba);
    // manager->PrintSegmentsInfo();
  }

  std::cout << "----------Overwrite----------" << std::endl;

  std::mt19937 gen(std::time(0));
  ZipfDistribution zipf(max_lba, 1.0);

  for (int i = 0; i < write_cnt; ++i) {
    int rand_lba = zipf(gen) - 1;
    manager->UserAppendBlock(rand_lba);
    // manager->PrintSegmentsInfo();
  }
}

}  // namespace logstore