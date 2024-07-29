#include "segment/segment_manager.h"
#include "gtest/gtest.h"
#include "select/greedy_select_segment.h"

#include <iostream>

namespace logstore {

TEST(SegmentManagerTest, SequenceWrite) {
  uint32_t segment_num = 5;
  uint32_t segment_capacity = 2;
  int32_t max_lba = segment_num * segment_capacity;
  pba_t l2p_map[max_lba];
  SegmentManager *manager = new SegmentManager(segment_num, segment_capacity);

  constexpr int32_t seq_num = 10;
  lba_t write_sequence[seq_num] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  // First, check all blocks are invalid initially
  for (pba_t pba = 0; pba < max_lba; pba++) {
    EXPECT_EQ(manager->IsValid(pba), false);
  }

  // Then, write the sequence
  for (int32_t i = 0; i < seq_num; i++) {
    lba_t lba = write_sequence[i];
    pba_t pba = manager->AllocateFreeBlock();
    l2p_map[lba] = pba;
    manager->MarkBlockValid(pba, lba);
  }

  // Check the mapping
  for (int i = 0; i < max_lba; i++) {
    pba_t pba = l2p_map[i];
    segment_id_t segment_id = manager->PBA2SegmentId(pba);
    off64_t offset = manager->PBA2SegmentOffset(pba);
    EXPECT_EQ(manager->IsValid(pba), true);
    EXPECT_EQ(segment_id, i / segment_capacity);
    EXPECT_EQ(offset, i % segment_capacity);
  }

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
  lba_t p2l_map[max_pba];
  SegmentManager *manager = new SegmentManager(segment_num, segment_capacity);

  // Clear the mapping
  for (int i = 0; i < max_lba; i++) {
    l2p_map[i] = INVALID_PBA;
  }

  for (int i = 0; i < max_pba; i++) {
    p2l_map[i] = INVALID_LBA;
  }

  // First, check all blocks are invalid initially
  for (pba_t pba = 0; pba < max_pba; pba++) {
    EXPECT_EQ(manager->IsValid(pba), false);
  }

  // Then, sequentially fill the blocks
  for (lba_t lba = 0; lba < max_lba; lba++) {
    pba_t pba = manager->AllocateFreeBlock();
    l2p_map[lba] = pba;
    p2l_map[pba] = lba;
    manager->MarkBlockValid(pba, lba);
  }

  // Only OP space is left
  EXPECT_EQ(manager->GetFreeSegmentRatio(), op_ratio);

  // Update lba 0 and 1
  lba_t update1[2] = {0, 1};
  for (int i = 0; i < 2; i++) {
    lba_t lba = update1[i];
    pba_t old_pba = l2p_map[lba];
    EXPECT_NE(old_pba, INVALID_PBA);
    manager->MarkBlockInvalid(old_pba);
    p2l_map[old_pba] = INVALID_LBA;

    pba_t new_pba = manager->AllocateFreeBlock();
    l2p_map[lba] = new_pba;
    p2l_map[new_pba] = lba;
    manager->MarkBlockValid(new_pba, lba);
  }

  // Only one segment is left
  EXPECT_EQ(manager->GetFreeSegmentRatio(), 1.0 / segment_num);

  Segment *victim = manager->SelectVictimSegment();
  EXPECT_EQ(victim->GetSegmentId(), 0);  // The first segment is the victim
  for (off64_t offset = 0; offset < segment_capacity; offset++) {
    EXPECT_EQ(victim->IsValid(offset), false);  // All blocks are invalid
  }

  // Move the victim to free list
  manager->GetSealedSegments().remove(victim);
  manager->GetFreeSegments().push_back(victim);

  // Now, the free segment ratio should be 2
  EXPECT_EQ(manager->GetFreeSegmentRatio(), 2.0 / segment_num);

  lba_t update2[2] = {2, 4};
  for (int i = 0; i < 2; i++) {
    lba_t lba = update2[i];
    pba_t old_pba = l2p_map[lba];
    EXPECT_NE(old_pba, INVALID_PBA);
    manager->MarkBlockInvalid(old_pba);
    p2l_map[old_pba] = INVALID_LBA;

    pba_t new_pba = manager->AllocateFreeBlock();
    l2p_map[lba] = new_pba;
    p2l_map[new_pba] = lba;
    manager->MarkBlockValid(new_pba, lba);
  }

  EXPECT_EQ(manager->GetFreeSegmentRatio(), 1.0 / segment_num);
}

}  // namespace logstore