#include "framework/request_scheduler.h"
#include "common/logger.h"

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
    if (request->is_write_) {
      controller_->WriteMultiBlock(request->buf_, request->slba_, request->len_);
    } else {
      controller_->ReadMultiBlock(request->buf_, request->slba_, request->len_);
    }
    // Signal to the request issuer that the request has been completed.
    request->callback_.set_value(true);
    while (controller_->GetFreeSegmentRatio() < controller_->GetOpRatio()) {
      LOG_DEBUG("Free segment ratio: %f, GC ratio: %f; Do GC", controller_->GetFreeSegmentRatio(),
                controller_->GetOpRatio());
      controller_->DoGC();
    }
  }
}

}  // namespace logstore