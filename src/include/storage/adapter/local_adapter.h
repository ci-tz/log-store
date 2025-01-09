#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include "storage/adapter/adapter.h"

namespace logstore {

class LocalAdapter : public Adapter {
 public:
  LocalAdapter(int32_t num, int32_t capacity);
  ~LocalAdapter() override;

  virtual uint64_t WriteBlock(const char *buf, pba_t pba) override;

  virtual uint64_t ReadBlock(char *buf, pba_t pba) override;

  virtual void EraseSegment(seg_id_t seg_id) override;

 private:
  seg_id_t GetSegmentId(pba_t pba);
  off64_t GetOffset(pba_t pba);
  void CreateSegment(int32_t id);
  void DestroySegment(int32_t id);
  std::unordered_map<int32_t, std::fstream> file_map_;  // segment id --> file stream
  std::unordered_map<int32_t, off64_t> offset_map_;     // segment id --> next offset
  const std::string dir_ = Config::GetInstance().localAdapterDir;
};

}  // namespace logstore
