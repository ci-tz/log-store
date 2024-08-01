#pragma once

#include "common/channel.h"
#include "framework/controller.h"

#include <future>
#include <optional>
#include <thread>

namespace logstore {

struct Request {
  bool is_write_;
  char *buf_;
  lba_t slba_;
  size_t len_;

  /** Callback used to signal to the request issuer when the request has been completed. */
  std::promise<bool> callback_;
};

class RequestScheduler {
 public:
  explicit RequestScheduler(Controller *controller);
  ~RequestScheduler();

  void Schedule(Request r);

  void StartWorkerThread();

  using SchedulerPromise = std::promise<bool>;
  SchedulerPromise CreatePromise() { return {}; };

 private:
  Controller *controller_;
  /** A shared queue to concurrently schedule and process requests.
   * When the DiskScheduler's destructor is called, `std::nullopt` is put into the queue to signal
   * to the background thread to stop execution. */
  Channel<std::optional<Request>> request_queue_;
  /** The background thread responsible for issuing scheduled requests to the disk manager. */
  std::optional<std::thread> background_thread_;
};

}  // namespace logstore