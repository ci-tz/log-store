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

  void WriteBlock(const char *buf, int32_t id, off64_t offset) override;

  void ReadBlock(char *buf, int32_t id, off64_t offset) override;

 private:
  void CreateSegment(int32_t id);
  void DestroySegment(int32_t id);
  std::unordered_map<int32_t, std::fstream> file_map_;
  std::unordered_map<int32_t, off64_t> offset_map_;
  const std::string dir_ = Config::GetInstance().localAdapterDir;
};

}  // namespace logstore
