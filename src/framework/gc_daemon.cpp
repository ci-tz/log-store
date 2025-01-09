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
    sm_ptr_->global_mutex_.lock();
    while (sm_ptr_->ShouldGc()) {
      std::cout << "GcDaemon: Doing GC\n";
      sm_ptr_->DoGc();
    }
    sm_ptr_->global_mutex_.unlock();
    sm_ptr_->cv_.notify_all();
  }
}

};  // namespace logstore