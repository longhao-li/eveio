#ifndef EVEIO_EVENTLOOP_THREAD_HPP
#define EVEIO_EVENTLOOP_THREAD_HPP

#include "eveio/EventLoop.hpp"
#include "eveio/SmartPtr.hpp"

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace eveio {

class EventLoopThread {
  EventLoop *loop;
  std::thread loop_thread;
  UniquePtr<std::mutex> mutex;
  UniquePtr<std::condition_variable> cond;
  std::function<void(EventLoop *)> init_callback;

public:
  EventLoopThread() noexcept;

  template <class Fn, class... Args>
  EventLoopThread(Fn &&fn, Args &&...args)
      : loop(nullptr),
        loop_thread(),
        mutex(MakeUnique<std::mutex>()),
        cond(MakeUnique<std::condition_variable>()),
        init_callback(std::bind(std::forward<Fn>(fn),
                                std::forward<Args>(args)...,
                                std::placeholders::_1)) {}

  EventLoopThread(const EventLoopThread &) = delete;
  EventLoopThread &operator=(const EventLoopThread &) = delete;

  EventLoopThread(EventLoopThread &&other) noexcept;
  EventLoopThread &operator=(EventLoopThread &&other) noexcept;

  ~EventLoopThread() noexcept;

  EventLoop *StartLoop() noexcept;

private:
  void Task() noexcept;
};

} // namespace eveio

#endif // EVEIO_EVENTLOOP_THREAD_HPP
