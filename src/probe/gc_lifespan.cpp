#include <iostream>

#include <map>
#include "probe/gc_lifespan.h"

namespace logstore {

void GcLifespan::MarkUserWrite(lba_t lba, int64_t write_time) {
  if (last_write_time_.find(lba) != last_write_time_.end()) {  // The lba is overwritten.
    int64_t last_write = last_write_time_[lba];
    int64_t lifespan = write_time - last_write;
    if (migrate_count_[lba] != 0) {
      int32_t mig_cnt = migrate_count_[lba];
      migrate_count_[lba] = 0;

      count_[mig_cnt]++;
      total_lifespan_[mig_cnt] += lifespan;
    }
  }
  last_write_time_[lba] = write_time;  // Update the last write time of lba[lba]
  wss_.insert(lba);
}

void GcLifespan::MarkGc(lba_t lba) { migrate_count_[lba]++; }

void GcLifespan::PrintCount() {
  // Sort the count_ by key
  std::map<int32_t, int32_t> sorted_count;  // migrate_count --> record_num
  for (auto it = count_.begin(); it != count_.end(); ++it) {
    sorted_count[it->first] = it->second;
  }

  // Print the sorted count_
  std::cout << "MigrateCount,RecordNum" << std::endl;
  for (const auto &pair : sorted_count) {
    printf("%d,%d\n", pair.first, pair.second);
  }
}

void GcLifespan::PrintAverageLifespan() {
  std::map<int32_t, double> sorted_avg_lifespan;
  int32_t wss = wss_.size();
  for (auto it = count_.begin(); it != count_.end(); ++it) {
    int32_t mig_cnt = it->first;
    int32_t record_num = it->second;
    double avg_lifespan = total_lifespan_[mig_cnt] * 1.0 / record_num / wss;
    sorted_avg_lifespan[mig_cnt] = avg_lifespan;
  }

  // Print the sorted average lifespan
  std::cout << "MigrateCount,AverageLifespan(x WSS)" << std::endl;
  for (const auto &pair : sorted_avg_lifespan) {
    printf("%d,%.2f\n", pair.first, pair.second);
  }
}

};  // namespace logstore