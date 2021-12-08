#ifndef EVEIO_EVENTLOOP_THREAD_HPP
#define EVEIO_EVENTLOOP_THREAD_HPP

#include "eveio/EventLoop.hpp"

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

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

class EventLoopThread {
  EventLoop *loop;
  std::thread loop_thread;
  std::mutex mutex;
  std::condition_variable cond;
  std::function<void(EventLoop *)> init_callback;

public:
  EventLoopThread() noexcept;

  template <class Fn, class... Args>
  EventLoopThread(Fn &&fn, Args &&...args)
      : loop(nullptr),
        loop_thread(),
        mutex(),
        cond(),
        init_callback(std::bind(std::forward<Fn>(fn),
                                std::forward<Args>(args)...,
                                std::placeholders::_1)) {}

  EventLoopThread(const EventLoopThread &) = delete;
  EventLoopThread &operator=(const EventLoopThread &) = delete;

  EventLoopThread(EventLoopThread &&other) = delete;
  EventLoopThread &operator=(EventLoopThread &&other) = delete;

  ~EventLoopThread() noexcept;

  EventLoop *StartLoop() noexcept;

private:
  void Task() noexcept;
};

} // namespace eveio

#endif // EVEIO_EVENTLOOP_THREAD_HPP
