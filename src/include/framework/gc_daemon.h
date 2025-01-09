#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "framework/segment_manager.h"

namespace logstore {

class GcDaemon {
 public:
  GcDaemon(std::shared_ptr<SegmentManager> sm_ptr);

  ~GcDaemon();

  void Stop() { running_ = false; }

 private:
  void daemonTask();

  std::shared_ptr<SegmentManager> sm_ptr_;
  std::thread daemonThread;    // 守护线程
  std::atomic<bool> running_;  // 标志位，控制线程运行状态
};

};  // namespace logstore