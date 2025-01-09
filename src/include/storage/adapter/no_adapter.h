#pragma once

#include "storage/adapter/adapter.h"

namespace logstore {

class NoAdapter : public Adapter {
 public:
  NoAdapter(int32_t num, int32_t capacity);
  ~NoAdapter() override = default;

  virtual uint64_t WriteBlock(const char *buf, pba_t pba) override;

  virtual uint64_t ReadBlock(char *buf, pba_t pba) override;

  virtual void EraseSegment(seg_id_t seg_id) override;
};

}  // namespace logstore