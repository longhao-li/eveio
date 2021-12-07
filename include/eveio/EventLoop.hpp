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
