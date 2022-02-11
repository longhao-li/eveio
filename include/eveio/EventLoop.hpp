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

#ifndef EVEIO_EVENTLOOP_HPP
#define EVEIO_EVENTLOOP_HPP

#include "eveio/Config.hpp"
#include "eveio/WakeupHandle.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace eveio {

class Channel;
class Poller;

class EventLoop {
  using ChannelList = std::vector<Channel *>;

  std::unique_ptr<Poller> poller;
  std::atomic_bool isLooping;
  std::atomic_bool isQuit;
  std::atomic_bool isHandlingEvent;

  WakeupHandle wakeupHandle;
  std::unique_ptr<Channel> wakeupChannel;

  threadid_t threadID;
  ChannelList activeChannels;

  std::vector<std::function<void()>> pendingFunc;
  std::mutex pendingFuncMutex;

public:
  /// Poller may throw SystemErrorException.
  /// Throw RuntimeErrorException if current thread already has one loop.
  EventLoop();
  ~EventLoop() noexcept;

  EventLoop(const EventLoop &) = delete;
  EventLoop &operator=(const EventLoop &) = delete;

  EventLoop(EventLoop &&) = delete;
  EventLoop &operator=(EventLoop &&) = delete;

  /// Thread safe method.
  /// Safe to call for multi-times.
  void Loop() noexcept;

  /// Thread safe method.
  /// Safe to call for multi-times.
  void Quit() noexcept;

  /// Let poller return immediately.
  void WakeUp() const noexcept { wakeupHandle.Trigger(); }

  /// Thread safe method.
  bool IsLooping() const noexcept {
    return isLooping.load(std::memory_order_relaxed);
  }

  /// Thread safe method.
  bool IsHandlingEvent() const noexcept {
    return isHandlingEvent.load(std::memory_order_relaxed);
  }

  /// Thread safe method.
  bool IsInLoopThread() const noexcept { return threadID == GetThreadID(); }

  /// Thread safe method.
  template <class Fn>
  void QueueInLoop(Fn &&fn) {
    std::lock_guard<std::mutex> lock(pendingFuncMutex);
    pendingFunc.emplace_back(std::forward<Fn>(fn));
  }

  /// Thread safe method.
  template <class Fn>
  void RunInLoop(Fn &&fn) {
    if (IsInLoopThread()) {
      fn();
    } else {
      QueueInLoop(std::forward<Fn>(fn));
      WakeUp();
    }
  }

  /// For Internal Usage
  /// Called by channel.
  void UpdateChannel(Channel *channel) const;

  /// For Internal Usage
  /// Called by channel.
  void UnregistChannel(Channel *channel) const;

  static EventLoop *GetCurrentThreadLoop() noexcept;

private:
  void DoPendingFunc() noexcept;
};

} // namespace eveio

#endif // EVEIO_EVENTLOOP_HPP
