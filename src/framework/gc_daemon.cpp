#include <mutex>

#include "framework/gc_daemon.h"

namespace logstore {

GcDaemon::GcDaemon(std::shared_ptr<SegmentManager> sm_ptr) : sm_ptr_(sm_ptr), running_(true) {
  daemonThread = std::thread(&GcDaemon::daemonTask, this);
  std::cout << "GC Daemon Thead Started\n";
}

GcDaemon::~GcDaemon() {
  Stop();

  if (daemonThread.joinable()) {
    daemonThread.join();
  }
  std::cout << "GC Daemon Thead Stopped\n";
}

void GcDaemon::daemonTask() {
  while (running_) {
    std::unique_lock<std::mutex> lock(sm_ptr_->global_mutex_);
    int32_t state = sm_ptr_->ShouldGc();
    if (state == 0) {
      lock.unlock();
      std::this_thread::sleep_for(std::chrono::microseconds(500));
    } else if (state == 1) {
      sm_ptr_->DoGc(false);
      lock.unlock();
    } else {
      sm_ptr_->DoGc(true);
      lock.unlock();
      sm_ptr_->cv_.notify_all();
    }
  }
}

};  // namespace logstore