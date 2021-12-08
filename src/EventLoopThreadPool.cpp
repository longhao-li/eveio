#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/EventLoopThread.hpp"

#include <spdlog/spdlog.h>

#include <cstdlib>

using namespace eveio;

eveio::EventLoopThreadPool::EventLoopThreadPool(size_t thread_num) noexcept
    : is_started(false),
      num_thread(thread_num > 0 ? thread_num : 1),
      next_loop(0),
      workers(),
      loops() {}

EventLoop *eveio::EventLoopThreadPool::GetNextLoop() noexcept {
  if (is_started.load(std::memory_order_relaxed))
    return loops[next_loop.fetch_add(1, std::memory_order_relaxed) %
                 loops.size()];
  return nullptr;
}

Vector<EventLoop *> eveio::EventLoopThreadPool::GetAllLoops() const noexcept {
  return loops;
}

void eveio::EventLoopThreadPool::Start() noexcept {
  if (is_started.exchange(true, std::memory_order_relaxed) == false) {
    if (num_thread <= 0) {
      SPDLOG_CRITICAL("num_thread is 0.");
      std::abort();
    }
    for (size_t i = 0; i < num_thread; ++i) {
      workers.emplace_back(MakeUnique<EventLoopThread>());
      loops.emplace_back(workers.back()->StartLoop());
    }
  }
}
