#ifndef EVEIO_EVENTLOOP_THREADPOOL_HPP
#define EVEIO_EVENTLOOP_THREADPOOL_HPP

#include "eveio/EventLoopThread.hpp"
#include "eveio/Vector.hpp"

#include <atomic>
#include <cassert>

namespace eveio {

class EventLoopThreadPool {
  std::atomic_bool is_started;
  size_t num_thread;
  std::atomic_size_t next_loop;
  Vector<EventLoopThread> workers;
  Vector<EventLoop *> loops;

public:
  EventLoopThreadPool(
      size_t thread_num = std::thread::hardware_concurrency()) noexcept;

  EventLoopThreadPool(const EventLoopThreadPool &) = delete;
  EventLoopThreadPool &operator=(const EventLoopThreadPool &) = delete;

  EventLoopThreadPool(EventLoopThreadPool &&) = delete;
  EventLoopThreadPool &operator=(EventLoopThreadPool &&) = delete;

  ~EventLoopThreadPool() noexcept = default;

  EventLoop *GetNextLoop() noexcept;
  Vector<EventLoop *> GetAllLoops() const noexcept;

  void SetThreadNum(size_t num) noexcept { num_thread = num; }

  bool IsStarted() const noexcept {
    return is_started.load(std::memory_order_relaxed);
  }

  void Start() noexcept;

  template <class Fn, class... Args>
  void Start(Fn &&init, Args &&...args) {
    if (is_started.exchange(true, std::memory_order_relaxed) == false) {
      assert(num_thread > 0);
      for (size_t i = 0; i < num_thread; ++i) {
        workers.emplace_back(std::forward<Fn>(init), std::forward<Fn>(args)...);
        loops.emplace_back(workers.back().StartLoop());
      }
    }
  }
};

} // namespace eveio

#endif // EVEIO_EVENTLOOP_THREADPOOL_HPP
