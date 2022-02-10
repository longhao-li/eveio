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

#ifndef EVEIO_EVENTLOOP_THREAD_HPP
#define EVEIO_EVENTLOOP_THREAD_HPP

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace eveio {

class Eventloop;

class EventloopThread {
  Eventloop *loop;
  std::thread loopThread;
  std::mutex loopMutex;
  std::condition_variable loopCond;
  std::function<void(Eventloop *)> initCallback;

public:
  EventloopThread() noexcept;
  ~EventloopThread() noexcept;

  EventloopThread(const EventloopThread &) = delete;
  EventloopThread &operator=(const EventloopThread &) = delete;

  EventloopThread(EventloopThread &&) = delete;
  EventloopThread &operator=(EventloopThread &&) = delete;

  template <class Fn>
  EventloopThread(Fn &&fn)
      : loop(nullptr),
        loopThread(),
        loopMutex(),
        loopCond(),
        initCallback(std::forward<Fn>(fn)) {}

  Eventloop *StartLoop() noexcept;

private:
  void Task() noexcept;
};

} // namespace eveio

#endif // EVEIO_EVENTLOOP_THREAD_HPP
