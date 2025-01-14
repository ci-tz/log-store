#include <boost/random/mersenne_twister.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "common/approx_zipf_distribution.h"
#include "common/config.h"
#include "common/logger.h"
#include "common/zipf_distribution.h"
#include "framework/gc_daemon.h"
#include "framework/segment_manager.h"
#include "gtest/gtest.h"
#include "select/selection_factory.h"

namespace logstore {

TEST(SegmentManagerTest, DISABLED_SepGC) {
  Config &config = Config::GetInstance();
  config.seg_num = 64;
  config.seg_cap = 131072;
  config.op = 0.25;
  config.selection = "Greedy";  // CostBenefit Greedy
  config.index_map = "Array";   // Array Hash
  config.adapter = "NoAdapter";

  int32_t max_lba = config.seg_num * (1 - config.op) * config.seg_cap;
  int32_t write_cnt = max_lba * 10;

  // 齐夫分布生成器
  int N = write_cnt;  // 最大支持值
  double s = 1.0;     // Zipf 分布参数（越大分布越陡峭）
  std::random_device rd;
  std::mt19937 gen(rd());
  ApproxZipfDistribution zipf(N, s);

  // 两种数据布局的比较
  config.placement = "NoPlacement";
  config.opened_segment_num = 1;
  std::shared_ptr<SegmentManager> no_place_manager = std::make_shared<SegmentManager>();

  config.placement = "SepGC";
  config.opened_segment_num = 2;
  std::shared_ptr<SegmentManager> sepgc_manager = std::make_shared<SegmentManager>();

  LOG_INFO("Start writing sequence...");
  LOG_INFO("It takes about 1 minute to complete...");
  for (int i = 0; i < write_cnt; ++i) {
    int32_t rand_lba = zipf(gen) % max_lba;
    no_place_manager->UserAppendBlock(rand_lba);
    sepgc_manager->UserAppendBlock(rand_lba);
  }
}

TEST(SegmentManagerTest, DISABLED_GcMigrate) {
  Config &config = Config::GetInstance();
  config.seg_num = 64;
  config.seg_cap = 1024;
  config.op = 0.25;
  config.placement = "NoPlacement";
  config.opened_segment_num = 1;
  config.selection = "Greedy";  // CostBenefit Greedy
  config.index_map = "Array";   // Array Hash
  config.adapter = "NoAdapter";

  int32_t max_lba = config.seg_num * (1 - config.op) * config.seg_cap;
  int32_t write_cnt = max_lba * 6;
  std::shared_ptr<SegmentManager> manager = std::make_shared<SegmentManager>();

  LOG_INFO("Warm up...");
  for (int lba = 0; lba < max_lba; ++lba) {
    manager->UserAppendBlock(lba);
  }

  // 齐夫分布生成器
  int N = write_cnt;  // 最大支持值
  double s = 1.0;     // Zipf 分布参数（越大分布越陡峭）
  std::random_device rd;
  std::mt19937 gen(rd());
  ApproxZipfDistribution zipf(N, s);

  LOG_INFO("Start writing sequence...");
  for (int i = 0; i < write_cnt; ++i) {
    int32_t rand_lba = zipf(gen) % max_lba;
    manager->UserAppendBlock(rand_lba);
  }
}

TEST(SegmentManagerTest, Play) {
  Config &config = Config::GetInstance();
  config.seg_num = 64;
  config.seg_cap = 131072;
  config.op = 0.25;
  config.placement = "SepBIT";  // SepGC SepBIT NoPlacement
  config.opened_segment_num = 6;
  config.selection = "Greedy";  // CostBenefit Greedy
  config.index_map = "Array";   // Array Hash
  config.adapter = "NoAdapter";

  int32_t max_lba = config.seg_num * (1 - config.op) * config.seg_cap;
  int32_t write_cnt = max_lba * 3;
  std::shared_ptr<SegmentManager> manager = std::make_shared<SegmentManager>();

  LOG_INFO("Warm up...");
  for (int lba = 0; lba < max_lba; ++lba) {
    manager->UserAppendBlock(lba);
  }

  // 齐夫分布生成器
  int N = write_cnt;  // 最大支持值
  double s = 1.0;     // Zipf 分布参数（越大分布越陡峭）
  std::random_device rd;
  std::mt19937 gen(rd());
  ApproxZipfDistribution zipf(N, s);

  LOG_INFO("Start writing sequence...");
  for (int i = 0; i < write_cnt; ++i) {
    int32_t rand_lba = zipf(gen) % max_lba;
    manager->UserAppendBlock(rand_lba);
  }
}

}  // namespace logstore