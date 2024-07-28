#include "storage/adapter/memory_adapter.h"
#include "gtest/gtest.h"
#include "common/macros.h"

namespace logstore {

TEST(MemoryAdapterTest, WriteBlock) {
  MemoryAdapter memory_adapter(10, 10);
  const char *str = "123456789";
  char write_buf[BLOCK_SIZE], read_buf[BLOCK_SIZE];
  memset(write_buf, 0, BLOCK_SIZE);
  memset(read_buf, 0, BLOCK_SIZE);
  memcpy(write_buf, str, strlen(str));
  memory_adapter.WriteBlock(write_buf, 0, 0);
  memory_adapter.ReadBlock(read_buf, 0, 0);
  EXPECT_STREQ(write_buf, read_buf);
}

}  // namespace logstore