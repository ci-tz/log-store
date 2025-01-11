#include <mutex>

#include "common/logger.h"
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
}

void GcDaemon::daemonTask() {
  while (running_) {
    std::unique_lock<std::mutex> lock(sm_ptr_->global_mutex_);
    int32_t state = sm_ptr_->ShouldGc();
    if (state == 0) {
      lock.unlock();
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } else if (state == 1) {
      sm_ptr_->DoGc(false);  // It may do nothing here.
      lock.unlock();
    } else {
      do {
        sm_ptr_->DoGc(true);
      } while (sm_ptr_->ShouldGc() == 2);
      lock.unlock();
      sm_ptr_->cv_.notify_all();
    }
  }
}

};  // namespace logstore