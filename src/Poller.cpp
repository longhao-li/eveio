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

#if defined(EVEIO_POLLER_HAS_KQUEUE)

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
