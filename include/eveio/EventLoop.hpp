#ifndef EVEIO_EVENTLOOP_HPP
#define EVEIO_EVENTLOOP_HPP

#include "eveio/Channel.hpp"
#include "eveio/Poller.hpp"
#include "eveio/SmartPtr.hpp"
#include "eveio/Vector.hpp"
#include "eveio/WakeupHandle.hpp"

#include <atomic>
#include <functional>
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

class EventLoop {
  typedef Vector<Channel *> ChannelList;

  Poller poller;

  std::atomic_bool is_looping;
  std::atomic_bool is_quit;

  volatile bool is_handling_event;

  WakeupHandle wakeup_handle;
  UniquePtr<Channel> wakeup_channel;

  std::thread::id t_id;

  ChannelList active_channels;

  mutable std::mutex mutex;
  Vector<std::function<void()>> pending_func;

public:
  EventLoop() noexcept;

  EventLoop(const EventLoop &) = delete;
  EventLoop &operator=(const EventLoop &) = delete;

  EventLoop(EventLoop &&) = delete;
  EventLoop &operator=(EventLoop &&) = delete;

  ~EventLoop() noexcept;

  void Loop() noexcept;
  void Quit() noexcept;

  void WakeUp() const noexcept { wakeup_handle.Trigger(); }

  bool IsRunning() const noexcept {
    return is_looping.load(std::memory_order_relaxed);
  }

  bool IsHandlingEvent() const noexcept { return is_handling_event; }

  bool IsInLoopThread() const noexcept {
    return t_id == std::this_thread::get_id();
  }

  template <class Fn, class... Args>
  void RunInLoop(Fn &&fn, Args &&...args) {
    if (IsInLoopThread()) {
      std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...)();
    } else {
      QueueInLoop(std::forward<Fn>(fn), std::forward<Args>(args)...);
      WakeUp();
    }
  }

  template <class Fn, class... Args>
  void QueueInLoop(Fn &&fn, Args &&...args) {
    std::lock_guard<std::mutex> lock(mutex);
    pending_func.emplace_back(
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
  }

  static EventLoop *CurrentThreadLoop() noexcept;

private:
  void DoPendingFunc() noexcept;

  friend void Channel::Update() noexcept;
  friend void Channel::Unregist() noexcept;
  friend bool Channel::IsRegisted() const noexcept;
};

} // namespace eveio

#endif // EVEIO_EVENTLOOP_HPP
