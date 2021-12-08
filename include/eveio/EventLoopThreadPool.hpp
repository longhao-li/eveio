#ifndef EVEIO_EVENTLOOP_THREADPOOL_HPP
#define EVEIO_EVENTLOOP_THREADPOOL_HPP

#include "eveio/EventLoopThread.hpp"
#include "eveio/SmartPtr.hpp"
#include "eveio/Vector.hpp"

#include <atomic>
#include <cassert>

/// Muduo - A reactor-based C++ network library for Linux
/// Copyright (c) 2010, Shuo Chen.  All rights reserved.
/// http://code.google.com/p/muduo/
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions
/// are met:
///
///   * Redistributions of source code must retain the above copyright
/// notice, this list of conditions and the following disclaimer.
///   * Redistributions in binary form must reproduce the above copyright
/// notice, this list of conditions and the following disclaimer in the
/// documentation and/or other materials provided with the distribution.
///   * Neither the name of Shuo Chen nor the names of other contributors
/// may be used to endorse or promote products derived from this software
/// without specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
/// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
/// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
/// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
/// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

namespace eveio {

class EventLoopThreadPool {
  std::atomic_bool is_started;
  size_t num_thread;
  std::atomic_size_t next_loop;
  Vector<UniquePtr<EventLoopThread>> workers;
  Vector<EventLoop *> loops;

public:
  // considering acceptor usually takes 1 thread, use hardware_concurrency() - 1
  // as default thread num.
  // it is OK to pass 0 here, num_thread will be set to 1.
  EventLoopThreadPool(size_t thread_num = std::thread::hardware_concurrency() -
                                          1) noexcept;

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
        workers.emplace_back(MakeUnique<EventLoopThread>(
            std::forward<Fn>(init), std::forward<Fn>(args)...));
        loops.emplace_back(workers.back()->StartLoop());
      }
    }
  }
};

} // namespace eveio

#endif // EVEIO_EVENTLOOP_THREADPOOL_HPP
