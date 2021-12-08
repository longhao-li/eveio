#include "eveio/Poller.hpp"
#include "eveio/Event.hpp"
#include "eveio/Handle.hpp"
#include "eveio/Time.hpp"

#include <fmt/ostream.h>

#include <spdlog/spdlog.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <thread>

using namespace eveio;

#if defined(EVEIO_POLLER_HAS_EPOLL)

enum {
  CHAN_STATE_INIT = Poller::ChannelInitPollState,
  CHAN_STATE_ADDED = 1,
  CHAN_STATE_DELETED = 2,
};

static constexpr Events MapEvent(uint32_t ep_event) noexcept {
  Events e = 0;
  if (ep_event & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    e |= event::ReadEvent;
  if (ep_event & (EPOLLOUT))
    e |= event::WriteEvent;
  if (ep_event & (EPOLLHUP))
    e |= event::CloseEvent;
  if (ep_event & (EPOLLERR))
    e |= event::ErrorEvent;
  return e;
}

static constexpr uint32_t UnmapEvent(Events e) noexcept {
  int res = 0;
  if (e & event::ReadEvent)
    res |= (EPOLLIN | EPOLLPRI | EPOLLRDHUP);
  if (e & event::WriteEvent)
    res |= (EPOLLOUT);
  if (e & event::CloseEvent)
    res |= (EPOLLHUP);
  if (e & event::ErrorEvent)
    res |= (EPOLLERR);
  return res;
}

static constexpr const int DefaultEventListCapacity = 16;

eveio::Poller::Poller() noexcept
    : ep_fd(::epoll_create1(EPOLL_CLOEXEC)), events(DefaultEventListCapacity) {
  if (ep_fd < 0) {
    SPDLOG_CRITICAL("failed to create epoll: {}.", std::strerror(errno));
    std::abort();
  }
}

eveio::Poller::~Poller() noexcept { ::close(ep_fd); }

Time eveio::Poller::Poll(Time::Milliseconds timeout,
                         ChannelList &active_channels) noexcept {
  int num_events = ::epoll_wait(ep_fd,
                                std::addressof(events[0]),
                                static_cast<int>(events.size()),
                                timeout.count());
  int saved_errno = errno;
  Time now = Time::Now();

  if (num_events > 0) {
    FillActiveChannels(num_events, active_channels);
    if (num_events == static_cast<int>(events.size()))
      events.resize(events.size() * 2);
  } else if (num_events < 0) {
    if (saved_errno != EINTR) {
      SPDLOG_ERROR(
          "epoll {} poll error: {}.", ep_fd, std::strerror(saved_errno));
      errno = saved_errno;
    }
  }
  return now;
}

void eveio::Poller::FillActiveChannels(
    int num_events, ChannelList &active_channels) const noexcept {
  for (int i = 0; i < num_events; i++) {
    Channel *channel = static_cast<Channel *>(events[i].data.ptr);
    channel->SetEventsToHandle(MapEvent(events[i].events));
    active_channels.push_back(channel);
  }
}

void eveio::Poller::UpdateChannel(Channel &chan) noexcept {
  const int32_t state = chan.GetPollState();
  if (state == CHAN_STATE_INIT || state == CHAN_STATE_DELETED) {
    if (state == CHAN_STATE_INIT)
      channels.emplace(std::make_pair(chan.GetHandle(), &chan));

    chan.SetPollState(CHAN_STATE_ADDED);
    Update(EPOLL_CTL_ADD, chan);
  } else {
    if (chan.IsNoneEvent()) {
      Update(EPOLL_CTL_DEL, chan);
      chan.SetPollState(CHAN_STATE_DELETED);
    } else {
      Update(EPOLL_CTL_MOD, chan);
    }
  }
}

void eveio::Poller::RemoveChannel(Channel &chan) noexcept {
  int32_t state = chan.GetPollState();
  channels.erase(chan.GetHandle());
  if (state == CHAN_STATE_ADDED)
    Update(EPOLL_CTL_DEL, chan);
  chan.SetPollState(CHAN_STATE_INIT);
}

void eveio::Poller::Update(int op, Channel &chan) noexcept {
  struct epoll_event event {};
  event.events = UnmapEvent(chan.ListeningEvents());
  event.data.ptr = &chan;
  if (::epoll_ctl(ep_fd, op, chan.GetHandle().native_handle(), &event) < 0) {
    SPDLOG_ERROR("epoll_ctl error. epfd: {}, operation: {}, error message: {}.",
                 ep_fd,
                 op,
                 std::strerror(errno));
  }
}

#elif defined(EVEIO_POLLER_HAS_KQUEUE)

enum {
  StateInit = Poller::ChannelInitPollState,
  StateAdded = 1,
  StateDeleted = 2,
};

enum {
  FlagError = EV_ERROR,
  FlagClose = EV_EOF,
};

eveio::Poller::Poller() noexcept
    : eveio::detail::PollerBase<Poller>(), kq_fd(::kqueue()), events(16) {
  if (kq_fd.native_handle() < 0) {
    SPDLOG_CRITICAL("failed to create kqueue: {}.", std::strerror(errno));
    std::abort();
  }
}

eveio::Poller::~Poller() noexcept { Handle::Close(kq_fd); }

Time eveio::Poller::Poll(Time::Milliseconds timeout,
                         ChannelList &active_channels) noexcept {
  struct timespec time_out {
    timeout.count() / 1000, (timeout.count() % 1000) * 1000000
  };

  int num_events = ::kevent(kq_fd.native_handle(),
                            nullptr,
                            0,
                            std::addressof(events[0]),
                            events.capacity(),
                            &time_out);
  int saved_errno = errno;
  Time now = Time::Now();

  if (num_events > 0) {
    FillActiveChannels(num_events, active_channels);
    if (num_events == static_cast<int>(events.capacity()))
      events.reserve(events.capacity() * 2);
  } else if (num_events < 0) {
    SPDLOG_CRITICAL("kqueue {} poll failed: {}.",
                    kq_fd.native_handle(),
                    std::strerror(saved_errno));
    std::abort();
  }
  return now;
}

void eveio::Poller::UpdateChannel(Channel &chan) noexcept {
  uint32_t extra_flag = 0;
  if (chan.GetPollState() == StateInit) {
    channels.emplace(std::make_pair(chan.GetHandle(), &chan));
    extra_flag = EV_ADD;
  }

  Update(event::ReadEvent, extra_flag, chan);
  Update(event::WriteEvent, extra_flag, chan);

  if (chan.IsNoneEvent())
    chan.SetPollState(StateDeleted);
  else
    chan.SetPollState(StateAdded);
}

void eveio::Poller::RemoveChannel(Channel &chan) noexcept {
  auto it = channels.find(chan.GetHandle());
  if (it != channels.end())
    channels.erase(it);
  chan.DisableAll();

  Update(event::ReadEvent, EV_DELETE, chan);
  Update(event::WriteEvent, EV_DELETE, chan);

  chan.SetPollState(StateInit);
}

void eveio::Poller::Update(uint32_t rw,
                           uint16_t extra_flag,
                           Channel &chan) noexcept {
  int16_t rw_flag = (rw == event::ReadEvent) ? EVFILT_READ : EVFILT_WRITE;
  uint16_t flag = (chan.ListeningEvents() & rw) ? EV_ENABLE : EV_DISABLE;

  struct kevent change;
  EV_SET(&change,
         chan.GetHandle().native_handle(),
         rw_flag,
         flag | extra_flag,
         0,
         0,
         &chan);

  struct timespec timeout {
    0, 0
  };

  if (::kevent(kq_fd.native_handle(), &change, 1, nullptr, 0, &timeout) < 0) {
    SPDLOG_ERROR(
        "kqueue update failed. Handle: {}, flags: {}, error message: {}.",
        chan.GetHandle().native_handle(),
        flag,
        std::strerror(errno));
  }
}

void eveio::Poller::FillActiveChannels(int num_events,
                                       ChannelList &active_channels) noexcept {
  for (int i = 0; i < num_events; ++i) {
    Channel *chan = static_cast<Channel *>(events[i].udata);
    assert(chan != nullptr);

    chan->SetEventsToHandle(event::NoneEvent);

    if (events[i].filter == EVFILT_READ)
      chan->AddEventsToHandle(event::ReadEvent);
    else if (events[i].filter == EVFILT_WRITE)
      chan->AddEventsToHandle(event::WriteEvent);

    if (events[i].flags & EV_ERROR)
      chan->AddEventsToHandle(event::ErrorEvent);
    if (events[i].flags & EV_EOF)
      chan->AddEventsToHandle(event::CloseEvent);

    // it is OK to add multi-times actually.
    if (active_channels.empty() || active_channels.back() != chan)
      active_channels.push_back(chan);
  }
}

#endif
