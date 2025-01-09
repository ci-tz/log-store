#include <iostream>

#include "common/config.h"
#include "framework/segment_manager.h"
#include "gtest/gtest.h"
#include "select/selection_factory.h"

namespace logstore {

TEST(SegmentManagerTest, SeqWrite) {
  constexpr int32_t seg_num = 4;
  constexpr int32_t seg_cap = 2;
  constexpr int32_t max_lba = (seg_num - 1) * seg_cap;

  SegmentManager *manager = new SegmentManager(seg_num, seg_cap);

  // write the sequence
  for (lba_t lba = 0; lba < max_lba; lba++) {
    manager->UserAppendBlock(lba);
    manager->PrintSegmentsInfo();
  }

  delete manager;
}

// TEST(SegmentManagerTest, DISABLED_RandWrite) {
//   constexpr uint32_t segment_num = 8;
//   constexpr uint32_t segment_capacity = 2;
//   constexpr double op_ratio = 0.25;
//   constexpr int32_t max_lba = (segment_num * (1 - op_ratio)) * segment_capacity;
//   constexpr int32_t max_pba = segment_num * segment_capacity;
//   pba_t l2p_map[max_lba];
//   std::shared_ptr<SelectSegment> select = std::make_shared<GreedySelectSegment>();
//   SegmentManager *manager = new SegmentManager(segment_num, segment_capacity, select);

//   // Clear the mapping
//   for (int i = 0; i < max_lba; i++) {
//     l2p_map[i] = INVALID_PBA;
//   }

//   // First, check all blocks are invalid initially
//   for (pba_t pba = 0; pba < max_pba; pba++) {
//     EXPECT_EQ(manager->IsValid(pba), false);
//   }

//   // Then, sequentially fill the blocks
//   for (lba_t lba = 0; lba < max_lba; lba++) {
//     pba_t pba = manager->AllocateFreeBlock();
//     l2p_map[lba] = pba;
//     manager->MarkBlockValid(pba, lba);
//   }
//   /**
//    *     0     1     2     3     4       5     6     7
//    *  |     |     |     |     |     |       |     |     |
//    *  | 0 1 | 2 3 | 4 5 | 6 7 | 8 9 | 10 11 | F F | F F |
//    *  |     |     |     |     |     |       |     |     |
//    */
//   EXPECT_EQ(manager->GetFreeSegmentRatio(), op_ratio);
//   for (lba_t lba = 0; lba < max_lba; lba++) {
//     pba_t pba = l2p_map[lba];
//     EXPECT_EQ(manager->IsValid(pba), true);
//   }

//   // Update lba 0 and 1
//   lba_t update1[2] = {0, 1};
//   pba_t old_pba[2];
//   for (int i = 0; i < 2; i++) {
//     lba_t lba = update1[i];
//     old_pba[i] = l2p_map[lba];
//     manager->MarkBlockInvalid(old_pba[i]);

//     pba_t new_pba = manager->AllocateFreeBlock();
//     l2p_map[lba] = new_pba;
//     manager->MarkBlockValid(new_pba, lba);
//   }

//   /**
//    *     0     1     2     3     4       5      6      7
//    *  |     |     |     |     |     |       |      |      |
//    *  | I I | 2 3 | 4 5 | 6 7 | 8 9 | 10 11 | 0  1 | F  F |
//    *  |     |     |     |     |     |       |      |      |
//    */
//   EXPECT_EQ(manager->GetFreeSegmentRatio(), 1.0 / segment_num);
//   EXPECT_EQ(manager->IsValid(old_pba[0]), false);
//   EXPECT_EQ(manager->IsValid(old_pba[1]), false);
//   EXPECT_EQ(manager->IsValid(l2p_map[0]), true);
//   EXPECT_EQ(manager->IsValid(l2p_map[1]), true);
//   EXPECT_EQ(manager->GetInvalidBlockNum(), 2);

//   // Modle the GC process
//   seg_id_t victim_id = manager->SelectVictimSegment();
//   // Only segment 0 should be selected and all blocks are invalid
//   EXPECT_EQ(victim_id, 0);
//   Segment *victim = manager->GetSegment(victim_id);
//   for (off64_t offset = 0; offset < segment_capacity; offset++) {
//     EXPECT_EQ(victim->IsValid(offset), false);
//   }
//   // Move the victim to free list and clear the metadata
//   manager->DoGCLeftWork(victim_id);

//   /**
//    *     0     1     2     3     4       5      6      7
//    *  |     |     |     |     |     |       |      |      |
//    *  | F F | 2 3 | 4 5 | 6 7 | 8 9 | 10 11 | 0  1 | F  F |
//    *  |     |     |     |     |     |       |      |      |
//    */
//   EXPECT_EQ(manager->GetFreeSegmentRatio(), 2.0 / segment_num);
//   EXPECT_EQ(manager->IsValid(old_pba[0]), false);
//   EXPECT_EQ(manager->IsValid(old_pba[1]), false);
//   EXPECT_EQ(manager->GetInvalidBlockNum(), 0);

//   lba_t update2[2] = {2, 4};
//   for (int i = 0; i < 2; i++) {
//     lba_t lba = update2[i];
//     old_pba[i] = l2p_map[lba];
//     manager->MarkBlockInvalid(old_pba[i]);

//     pba_t new_pba = manager->AllocateFreeBlock();
//     l2p_map[lba] = new_pba;
//     manager->MarkBlockValid(new_pba, lba);
//   }
//   /**
//    *     0     1     2     3     4       5      6      7
//    *  |     |     |     |     |     |       |      |      |
//    *  | F F | I 3 | I 5 | 6 7 | 8 9 | 10 11 | 0  1 | 2  4 |
//    *  |     |     |     |     |     |       |      |      |
//    */

//   EXPECT_EQ(manager->GetFreeSegmentRatio(), 1.0 / segment_num);
//   EXPECT_EQ(manager->IsValid(old_pba[0]), false);
//   EXPECT_EQ(manager->IsValid(old_pba[1]), false);
//   EXPECT_EQ(manager->IsValid(l2p_map[2]), true);
//   EXPECT_EQ(manager->IsValid(l2p_map[4]), true);
//   EXPECT_EQ(manager->GetInvalidBlockNum(), 2);

//   delete manager;
// }

}  // namespace logstore