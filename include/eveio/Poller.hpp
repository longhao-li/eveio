#ifndef EVEIO_POLLER_HPP
#define EVEIO_POLLER_HPP

#include "eveio/Channel.hpp"
#include "eveio/Event.hpp"
#include "eveio/Map.hpp"
#include "eveio/Vector.hpp"

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
namespace detail {

template <class T>
class PollerBase {
protected:
  Map<Handle, Channel *> channels;

public:
  typedef Vector<Channel *> ChannelList;

  PollerBase() noexcept = default;

  PollerBase(const PollerBase &) = delete;
  PollerBase &operator=(const PollerBase &) = delete;

  PollerBase(PollerBase &&) noexcept = default;
  PollerBase &operator=(PollerBase &&) noexcept = default;

  ~PollerBase() noexcept = default;

  template <class Dur>
  Time Poll(Dur timeout, ChannelList &active_channels) noexcept {
    return static_cast<T *>(this)->Poll(
        Time::duration_cast<Time::Milliseconds>(timeout), active_channels);
  }

  void UpdateChannel(Channel &chan) noexcept {
    static_cast<T *>(this)->UpdateChannel(chan);
  }

  void RemoveChannel(Channel &chan) noexcept {
    static_cast<T *>(this)->RemoveChannel(chan);
  }

  bool HasChannel(const Channel &chan) noexcept {
    auto it = channels.find(chan.GetHandle());
    return (it != channels.end() && it->second == &chan);
  }
};

} // namespace detail
} // namespace eveio

#if defined(EVEIO_POLLER_HAS_EPOLL)

#  include <sys/epoll.h>

namespace eveio {

class Poller : public detail::PollerBase<Poller> {
  int ep_fd;
  Vector<struct epoll_event> events;

public:
  static constexpr const int32_t ChannelInitPollState = -1;

  Poller() noexcept;
  ~Poller() noexcept;

  Time Poll(Time::Milliseconds timeout, ChannelList &active_channels) noexcept;

  void UpdateChannel(Channel &chan) noexcept;
  void RemoveChannel(Channel &chan) noexcept;

private:
  void Update(int op, Channel &chan) noexcept;
  void FillActiveChannels(int num_events,
                          ChannelList &active_channels) const noexcept;
};

} // namespace eveio

#elif defined(EVEIO_POLLER_HAS_KQUEUE)

#  include <sys/event.h>

namespace eveio {

class Poller : public detail::PollerBase<Poller> {
  Handle kq_fd;
  Vector<struct kevent> events;

public:
  static constexpr const int32_t ChannelInitPollState = -1;

  Poller() noexcept;
  ~Poller() noexcept;

  Time Poll(Time::Milliseconds timeout, ChannelList &active_channels) noexcept;

  void UpdateChannel(Channel &chan) noexcept;
  void RemoveChannel(Channel &chan) noexcept;

private:
  void Update(uint32_t rw, uint16_t extra_flag, Channel &chan) noexcept;
  void FillActiveChannels(int num_events,
                          ChannelList &active_channels) noexcept;
};

} // namespace eveio
#endif

#endif // EVEIO_POLLER_HPP
