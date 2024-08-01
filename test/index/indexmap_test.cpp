#include "gtest/gtest.h"
#include "index/array_indexmap.h"
#include "index/hash_indexmap.h"

namespace logstore {

TEST(IndexMapTest, DISABLED_HashIndexMapTest) {
  int32_t max_size = 10;
  HashIndexMap index_map(max_size);
  // Check that all LBAs are INVALID_PBA initially
  for (int i = 0; i < max_size; i++) {
    EXPECT_EQ(index_map.Query(i), INVALID_PBA);
  }
  // Update all LBAs to a valid PBA
  for (int i = 0; i < max_size; i++) {
    index_map.Update(i, i);
  }
  // Check that all LBAs are updated to the correct PBA
  for (int i = 0; i < max_size; i++) {
    EXPECT_EQ(index_map.Query(i), i);
  }
}

TEST(IndexMapTest, DISABLED_ArrayIndexMapTest) {
  int32_t max_size = 10;
  ArrayIndexMap index_map(max_size);
  // Check that all LBAs are INVALID_PBA initially
  for (int i = 0; i < max_size; i++) {
    EXPECT_EQ(index_map.Query(i), INVALID_PBA);
  }
  // Update all LBAs to a valid PBA
  for (int i = 0; i < max_size; i++) {
    index_map.Update(i, i);
  }
  // Check that all LBAs are updated to the correct PBA
  for (int i = 0; i < max_size; i++) {
    EXPECT_EQ(index_map.Query(i), i);
  }
}

}  // namespace logstore