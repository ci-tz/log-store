#include "segment/segment_manager.h"
#include "gtest/gtest.h"
#include "select/greedy_select_segment.h"

#include <iostream>

namespace logstore {

TEST(SegmentManagerTest, SequenceWrite) {
  constexpr int32_t segment_num = 8;
  constexpr int32_t segment_capacity = 2;
  constexpr int32_t max_lba = segment_num * segment_capacity;
  constexpr int32_t max_pba = segment_num * segment_capacity;
  pba_t l2p_map[max_lba];
  SegmentManager *manager = new SegmentManager(segment_num, segment_capacity);

  // First, check all blocks are invalid initially
  for (pba_t pba = 0; pba < max_pba; pba++) {
    EXPECT_EQ(manager->IsValid(pba), false);
  }

  // Then, write the sequence
  for (lba_t lba = 0; lba < max_lba; lba++) {
    pba_t pba = manager->AllocateFreeBlock();
    l2p_map[lba] = pba;
    manager->MarkBlockValid(pba, lba);
  }

  // Check the mapping
  for (lba_t lba = 0; lba < max_lba; lba++) {
    pba_t pba = l2p_map[lba];
    EXPECT_EQ(manager->IsValid(pba), true);
  }

  // std::cout << "After sequential write" << std::endl;
  // manager->PrintSegmentsInfo();

  // The free segment ratio should be 0
  EXPECT_EQ(manager->GetFreeSegmentRatio(), 0);

  delete manager;
}

TEST(SegmentManagerTest, RandWrite) {
  constexpr uint32_t segment_num = 8;
  constexpr uint32_t segment_capacity = 2;
  constexpr double op_ratio = 0.25;
  constexpr int32_t max_lba = (segment_num * (1 - op_ratio)) * segment_capacity;
  constexpr int32_t max_pba = segment_num * segment_capacity;
  pba_t l2p_map[max_lba];
  SegmentManager *manager = new SegmentManager(segment_num, segment_capacity);

  // Clear the mapping
  for (int i = 0; i < max_lba; i++) {
    l2p_map[i] = INVALID_PBA;
  }

  // First, check all blocks are invalid initially
  for (pba_t pba = 0; pba < max_pba; pba++) {
    EXPECT_EQ(manager->IsValid(pba), false);
  }

  // Then, sequentially fill the blocks
  for (lba_t lba = 0; lba < max_lba; lba++) {
    pba_t pba = manager->AllocateFreeBlock();
    l2p_map[lba] = pba;
    manager->MarkBlockValid(pba, lba);
  }

  // Only OP space is left
  EXPECT_EQ(manager->GetFreeSegmentRatio(), op_ratio);
  // std::cout << "After sequential write" << std::endl;
  // manager->PrintSegmentsInfo();

  // Update lba 0 and 1
  lba_t update1[2] = {0, 1};
  for (int i = 0; i < 2; i++) {
    lba_t lba = update1[i];
    pba_t old_pba = l2p_map[lba];
    manager->MarkBlockInvalid(old_pba);

    pba_t new_pba = manager->AllocateFreeBlock();
    l2p_map[lba] = new_pba;
    manager->MarkBlockValid(new_pba, lba);
  }
  // Only one segment is left
  EXPECT_EQ(manager->GetFreeSegmentRatio(), 1.0 / segment_num);
  // std::cout << "After first update" << std::endl;
  // manager->PrintSegmentsInfo();

  seg_id_t victim_id = manager->SelectVictimSegment();
  EXPECT_EQ(victim_id, 0);
  Segment *victim = manager->GetSegment(victim_id);
  for (off64_t offset = 0; offset < segment_capacity; offset++) {
    EXPECT_EQ(victim->IsValid(offset), false);  // All blocks are invalid
  }
  // Move the victim to free list and clear the metadata
  manager->DoGCLeftWork(victim_id);

  // Now, the free segment ratio should be 2
  EXPECT_EQ(manager->GetFreeSegmentRatio(), 2.0 / segment_num);
  // std::cout << "After GC" << std::endl;
  // manager->PrintSegmentsInfo();

  lba_t update2[2] = {2, 4};
  for (int i = 0; i < 2; i++) {
    lba_t lba = update2[i];
    pba_t old_pba = l2p_map[lba];
    manager->MarkBlockInvalid(old_pba);

    pba_t new_pba = manager->AllocateFreeBlock();
    l2p_map[lba] = new_pba;
    manager->MarkBlockValid(new_pba, lba);
  }
  EXPECT_EQ(manager->GetFreeSegmentRatio(), 1.0 / segment_num);
  // std::cout << "After second update" << std::endl;
  // manager->PrintSegmentsInfo();
}

}  // namespace logstore