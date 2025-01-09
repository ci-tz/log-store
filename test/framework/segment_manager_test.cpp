#include <iostream>
#include <random>

#include "common/config.h"
#include "framework/gc_daemon.h"
#include "framework/segment_manager.h"
#include "gtest/gtest.h"
#include "select/selection_factory.h"

namespace logstore {

TEST(SegmentManagerTest, SeqWrite) {
  Config &config = Config::GetInstance();
  config.placement = "NoPlacement";
  config.adapter = "NoAdapter";
  config.selection = "Greedy";
  config.opened_segment_num = 1;

  int32_t seg_num = 16;
  int32_t seg_cap = 2;
  double op = 0.25;
  int32_t max_lba = seg_num * (1 - op) * seg_cap;

  std::shared_ptr<SegmentManager> manager = std::make_shared<SegmentManager>(seg_num, seg_cap, op);

  std::shared_ptr<GcDaemon> gc_daemon = std::make_shared<GcDaemon>(manager);

  // write the sequence
  // for (lba_t lba = 0; lba < max_lba; lba++) {
  //   manager->UserAppendBlock(lba);
  //   manager->PrintSegmentsInfo();
  // }
  // std::cout << "----------Overwrite----------" << std::endl;

  std::random_device rd;
  std::mt19937 gen(rd());  // 使用梅森旋转算法生成伪随机数
  std::uniform_int_distribution<> dis(0, max_lba);
  int32_t write_cnt = 2000;
  for (auto i = 0; i < write_cnt; i++) {
    int32_t rand_lba = dis(gen);
    std::cout << "Write lba: " << rand_lba << std::endl;
    manager->UserAppendBlock(rand_lba);
    manager->PrintSegmentsInfo();
  }
}

}  // namespace logstore