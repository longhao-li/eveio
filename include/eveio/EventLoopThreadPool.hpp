/// Copyright (c) 2021 Li Longhao
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.

#ifndef EVEIO_EVENTLOOP_THREAD_POOL_HPP
#define EVEIO_EVENTLOOP_THREAD_POOL_HPP

#include "eveio/EventloopThread.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

namespace eveio {

class Eventloop;
class EventloopThread;

class EventloopThreadPool {
  std::atomic_bool isStarted;
  std::atomic_size_t numThread;
  std::atomic_size_t nextLoop;
  std::vector<std::unique_ptr<EventloopThread>> workers;
  std::vector<Eventloop *> loops;

public:
  EventloopThreadPool(
      size_t threadNum = std::thread::hardware_concurrency()) noexcept;
  ~EventloopThreadPool() noexcept = default;

  EventloopThreadPool(const EventloopThreadPool &) = delete;
  EventloopThreadPool &operator=(const EventloopThreadPool &) = delete;

  EventloopThreadPool(EventloopThreadPool &&) = delete;
  EventloopThreadPool &operator=(EventloopThreadPool &&) = delete;

  Eventloop *GetNextLoop() noexcept;

  const std::vector<Eventloop *> &GetAllLoops() const noexcept { return loops; }

  void SetThreadNum(size_t num) noexcept {
    numThread.store(num, std::memory_order_relaxed);
  }

  void AddLoop(Eventloop *loop) noexcept { loops.push_back(loop); }

  void Start() noexcept;

  template <class InitFn>
  void Start(InitFn &&loopInitFunc) {
    if (isStarted.exchange(true, std::memory_order_relaxed)) {
      return;
    }

    assert(numThread > 0);
    for (size_t i = 0; i < numThread; ++i) {
      workers.emplace_back(
          new EventloopThread(std::forward<InitFn>(loopInitFunc)));
      loops.emplace_back(workers.back()->StartLoop());
    }
  }
};

} // namespace eveio

#endif // EVEIO_EVENTLOOP_THREAD_POOL_HPP
