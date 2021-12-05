#include "eveio/EventLoopThreadPool.hpp"

#include <spdlog/spdlog.h>

#include <cstdlib>

eveio::EventLoopThreadPool::EventLoopThreadPool(size_t thread_num) noexcept
    : is_started(false),
      num_thread(thread_num),
      next_loop(0),
      workers(),
      loops(),
      mutex() {}

eveio::EventLoop *eveio::EventLoopThreadPool::GetNextLoop() noexcept {
  if (is_started.load(std::memory_order_relaxed)) {
    std::lock_guard<std::mutex> lock(mutex);
    EventLoop *res = loops[next_loop];
    next_loop = (next_loop + 1) % loops.size();
    return res;
  }
  return nullptr;
}

std::vector<eveio::EventLoop *>
eveio::EventLoopThreadPool::GetAllLoops() const noexcept {
  return loops;
}

void eveio::EventLoopThreadPool::Start() noexcept {
  if (is_started.exchange(true, std::memory_order_relaxed) == false) {
    if (num_thread <= 0) {
      SPDLOG_CRITICAL("num_thread is 0.");
      std::abort();
    }
    for (size_t i = 0; i < num_thread; ++i) {
      workers.emplace_back();
      loops.emplace_back(workers.back().StartLoop());
    }
  }
}
