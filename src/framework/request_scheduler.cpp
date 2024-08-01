#include "framework/request_scheduler.h"

namespace logstore {

RequestScheduler::RequestScheduler(Controller *controller) : controller_(controller) {
  background_thread_.emplace([&] { StartWorkerThread(); });
}

RequestScheduler::~RequestScheduler() {
  // Signal to the background thread to stop execution.
  request_queue_.Put(std::nullopt);
  background_thread_->join();
}

void RequestScheduler::Schedule(Request r) { request_queue_.Put(std::move(r)); }

void RequestScheduler::StartWorkerThread() {
  while (true) {
    auto request = request_queue_.Get();
    if (!request.has_value()) {
      break;
    }
    // TODO: Implement request processing here.
  }
}

}  // namespace logstore