#include <iostream>
#include <memory>
#include <vector>

#include "framework/phy_segment.h"
#include "framework/segment.h"
#include "gtest/gtest.h"

namespace logstore {

TEST(SubSegmentTest, Play) {
  int32_t num = 4;
  int32_t cap = 8;
  std::vector<std::shared_ptr<PhySegment>> phy_seg_ptrs;
  for (int i = 0; i < num; i++) {
    auto ptr = std::make_shared<PhySegment>(i, i * cap, cap);
    phy_seg_ptrs.emplace_back(ptr);
  }

  for (const auto &ptr : phy_seg_ptrs) {
    std::cout << *ptr << std::endl;
  }

  for (int i = 0; i < num; i++) {
    auto &sub_segs = phy_seg_ptrs[i]->OpenAs(i);
    for (auto &seg : sub_segs) {
      std::cout << *seg << std::endl;
    }
  }
  std::cout << "done" << std::endl;
}

};  // namespace logstore