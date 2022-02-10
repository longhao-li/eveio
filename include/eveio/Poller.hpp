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

#ifndef EVEIO_POLLER_HPP
#define EVEIO_POLLER_HPP

#include "eveio/Config.hpp"

#if EVEIO_OS_WIN32
#  define EVEIO_POLLER_USE_POLL 1
#elif EVEIO_OS_DARWIN || EVEIO_OS_FREEBSD
#  define EVEIO_POLLER_USE_KQUEUE 1
#elif EVEIO_OS_LINUX
#  define EVEIO_POLLER_USE_EPOLL 1
#endif

#include <chrono>
#include <cstdint>
#include <vector>
#if EVEIO_POLLER_USE_POLL
#elif EVEIO_POLLER_USE_KQUEUE
#  include <sys/event.h>
#elif EVEIO_POLLER_USE_EPOLL
#  include <sys/epoll.h>
#endif

namespace eveio {

class Channel;

#if EVEIO_POLLER_USE_POLL
#elif EVEIO_POLLER_USE_KQUEUE
class Poller {
  using ChannelList = std::vector<Channel *>;

  int kqfd;
  std::vector<struct kevent> events;

public:
  static constexpr const int32_t ChannelInitPollState = -1;

  Poller();
  ~Poller() noexcept;

  Poller(const Poller &) = delete;
  Poller &operator=(const Poller &) = delete;

  Poller(Poller &&) = delete;
  Poller &operator=(Poller &&) = delete;

  /// Throw SystemErrorException if any error occurs.
  std::chrono::system_clock::time_point Poll(std::chrono::milliseconds timeout,
                                             ChannelList &activeChannels);

  /// Throw SystemErrorException if any error occurs.
  void UpdateChannel(Channel *channel);

  /// Throw SystemErrorException if any error occurs.
  void UnregistChannel(Channel *channel);

private:
  void Update(uint32_t rw, uint16_t extFlag, Channel *channel);
  void FillActiveChannels(int numEvents, ChannelList &activeChannels) noexcept;
};
#elif EVEIO_POLLER_USE_EPOLL
class Poller {
  using ChannelList = std::vector<Channel *>;

  int epfd;
  std::vector<struct epoll_event> events;

public:
  static constexpr const int32_t ChannelInitPollState = -1;

  Poller();
  ~Poller() noexcept;

  Poller(const Poller &) = delete;
  Poller &operator=(const Poller &) = delete;

  Poller(Poller &&) = delete;
  Poller &operator=(Poller &&) = delete;

  /// Throw SystemErrorException if any error occurs.
  std::chrono::system_clock::time_point Poll(std::chrono::milliseconds timeout,
                                             ChannelList &activeChannels);

  /// Throw SystemErrorException if any error occurs.
  void UpdateChannel(Channel *channel);

  /// Throw SystemErrorException if any error occurs.
  void UnregistChannel(Channel *channel);

private:
  void Update(int op, Channel *channel);
  void FillActiveChannels(int numEvents,
                          ChannelList &activeChannels) const noexcept;
};
#endif

} // namespace eveio

#endif // EVEIO_POLLER_HPP
