#ifndef EVEIO_POLLER_HPP
#define EVEIO_POLLER_HPP

#include "eveio/Channel.hpp"
#include "eveio/Event.hpp"
#include "eveio/Map.hpp"
#include "eveio/Vector.hpp"

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
