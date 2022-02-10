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

#include "eveio/Poller.hpp"
#include "eveio/Channel.hpp"
#include "eveio/Event.hpp"
#include "eveio/Exception.hpp"

#include <cassert>
#include <chrono>
#include <cstring>

using namespace eveio;

#if EVEIO_POLLER_USE_POLL
#elif EVEIO_POLLER_USE_KQUEUE

enum {
  POLLER_STATE_INIT = Poller::ChannelInitPollState,
  POLLER_STATE_ADDED = 1,
  POLLER_STATE_DELETED = 2,
};

enum {
  POLLER_FLAG_ERROR = EV_ERROR,
  POLLER_FLAG_CLOSE = EV_EOF,
};

eveio::Poller::Poller() : kqfd(::kqueue()), events(16) {
  if (kqfd < 0) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Fained to create kqueue fd: ") +
                                   std::strerror(errno));
  }
}

eveio::Poller::~Poller() noexcept { ::close(kqfd); }

std::chrono::system_clock::time_point
eveio::Poller::Poll(std::chrono::milliseconds timeout,
                    ChannelList &activeChannels) {
  struct timespec timeOut {
    timeout.count() / 1000, (timeout.count() % 1000) * 1000 * 1000
  };

  int numEvents = ::kevent(kqfd,
                           nullptr,
                           0,
                           std::addressof(events[0]),
                           static_cast<int>(events.capacity()),
                           &timeOut);
  auto now = std::chrono::system_clock::now();

  if (numEvents > 0) {
    FillActiveChannels(numEvents, activeChannels);
    if (numEvents == static_cast<int>(events.capacity())) {
      events.reserve(events.capacity() * 2);
    }
  } else if (numEvents < 0) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Failed to poll events: ") +
                                   std::strerror(errno));
  }

  return now;
}

void eveio::Poller::UpdateChannel(Channel *channel) {
  uint16_t extFlag = 0;
  if (channel->GetPollerState() == POLLER_STATE_INIT) {
    extFlag = EV_ADD;
  }

  Update(EVENT_READ, extFlag, channel);
  Update(EVENT_WRITE, extFlag, channel);

  if (channel->IsNoneEvent()) {
    channel->SetPollerState(POLLER_STATE_DELETED);
  } else {
    channel->SetPollerState(POLLER_STATE_ADDED);
  }
}

void eveio::Poller::UnregistChannel(Channel *channel) {
  channel->DisableAll();
  Update(EVENT_READ, EV_DELETE, channel);
  Update(EVENT_WRITE, EV_DELETE, channel);
  channel->SetPollerState(POLLER_STATE_INIT);
}

void eveio::Poller::Update(uint32_t rw, uint16_t extFlag, Channel *channel) {
  int16_t rwFlag = (rw == EVENT_READ) ? EVFILT_READ : EVFILT_WRITE;
  uint16_t flag = (channel->EventsListening() & rw) ? EV_ENABLE : EV_DISABLE;

  struct kevent change;
  EV_SET(&change, channel->GetHandle(), rwFlag, flag | extFlag, 0, 0, channel);

  struct timespec timeout {};

  if (::kevent(kqfd, &change, 1, nullptr, 0, &timeout) < 0) {
    auto info = String("Failed to update channel ") +
                std::to_string(channel->GetHandle()) +
                ". Flags: " + std::to_string(flag | extFlag) +
                ", fd: " + std::to_string(channel->GetHandle()) + ", " +
                std::strerror(errno);
    throw SystemErrorException(
        __FILENAME__, __LINE__, __func__, std::move(info));
  }
}

void eveio::Poller::FillActiveChannels(int numEvents,
                                       ChannelList &activeChannels) noexcept {
  for (size_t i = 0; i < static_cast<size_t>(numEvents); ++i) {
    auto chan = static_cast<Channel *>(events[i].udata);
    assert(chan != nullptr);

    chan->SetEventsToHandle(EVENT_NONE);
    if (events[i].filter == EVFILT_READ) {
      chan->AddEventsToHandle(EVENT_READ);
    } else if (events[i].filter == EVFILT_WRITE) {
      chan->AddEventsToHandle(EVENT_WRITE);
    }

    if (events[i].flags & EV_ERROR) {
      chan->AddEventsToHandle(EVENT_ERROR);
    }
    if (events[i].flags & EV_EOF) {
      chan->AddEventsToHandle(EVENT_CLOSE);
    }

    if (activeChannels.empty() || activeChannels.back() != chan) {
      activeChannels.push_back(chan);
    }
  }
}

#elif EVEIO_POLLER_USE_EPOLL

enum {
  POLLER_STATE_INIT = Poller::ChannelInitPollState,
  POLLER_STATE_ADDED = 1,
  POLLER_STATE_DELETED = 2,
};

static Event MapEvent(uint32_t epEvent) noexcept {
  Event e = 0;

  if (epEvent & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    e |= EVENT_READ;
  }

  if (epEvent & (EPOLLOUT)) {
    e |= EVENT_WRITE;
  }

  if (epEvent & (EPOLLHUP)) {
    e |= EVENT_CLOSE;
  }

  if (epEvent & (EPOLLERR)) {
    e |= EVENT_ERROR;
  }

  return e;
}

static uint32_t UnmapEvent(Event e) noexcept {
  uint32_t res = 0;
  if (e & EVENT_READ) {
    res |= (EPOLLIN | EPOLLPRI | EPOLLRDHUP);
  }

  if (e & EVENT_WRITE) {
    res |= (EPOLLOUT);
  }

  if (e & EVENT_CLOSE) {
    res |= (EPOLLHUP);
  }

  if (e & EVENT_ERROR) {
    res |= (EPOLLERR);
  }

  return res;
}

eveio::Poller::Poller() : epfd(::epoll_create1(EPOLL_CLOEXEC)), events(16) {
  if (epfd < 0) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Fained to create epoll: ") +
                                   std::strerror(errno));
  }
}

eveio::Poller::~Poller() noexcept { ::close(epfd); }

std::chrono::system_clock::time_point
eveio::Poller::Poll(std::chrono::milliseconds timeout,
                    ChannelList &activeChannels) {
  int numEvents = ::epoll_wait(epfd,
                               std::addressof(events[0]),
                               static_cast<int>(events.capacity()),
                               static_cast<int>(timeout.count()));
  auto now = std::chrono::system_clock::now();

  if (numEvents > 0) {
    FillActiveChannels(numEvents, activeChannels);
    if (numEvents == static_cast<int>(events.capacity())) {
      events.reserve(events.capacity() * 2);
    }
  } else if (numEvents < 0) {
    if (errno != EINTR) {
      throw SystemErrorException(__FILENAME__,
                                 __LINE__,
                                 __func__,
                                 String("Failed to poll events: ") +
                                     std::strerror(errno));
    }
  }
  return now;
}

void eveio::Poller::FillActiveChannels(
    int numEvents, ChannelList &activeChannels) const noexcept {
  for (size_t i = 0; i < static_cast<size_t>(numEvents); ++i) {
    auto channel = static_cast<Channel *>(events[i].data.ptr);
    channel->SetEventsToHandle(MapEvent(events[i].events));
    activeChannels.push_back(channel);
  }
}

void eveio::Poller::UpdateChannel(Channel *channel) {
  const auto state = channel->GetPollerState();
  if (state == POLLER_STATE_INIT || state == POLLER_STATE_DELETED) {
    if (!channel->IsNoneEvent()) {
      channel->SetPollerState(POLLER_STATE_ADDED);
      Update(EPOLL_CTL_ADD, channel);
    }
  } else {
    if (channel->IsNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->SetPollerState(POLLER_STATE_DELETED);
    } else {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

void eveio::Poller::UnregistChannel(Channel *channel) {
  const auto state = channel->GetPollerState();
  if (state == POLLER_STATE_ADDED) {
    Update(EPOLL_CTL_DEL, channel);
  }
  channel->SetPollerState(POLLER_STATE_INIT);
}

void eveio::Poller::Update(int op, Channel *channel) {
  struct epoll_event event {};
  event.events = UnmapEvent(channel->EventsListening());
  event.data.ptr = channel;
  if (::epoll_ctl(epfd, op, channel->GetHandle(), &event) < 0) {
    auto info = String("Failed to update channel ") +
                std::to_string(channel->GetHandle()) +
                ". Operation: " + std::to_string(op) +
                ", fd: " + std::to_string(channel->GetHandle()) + ", " +
                std::strerror(errno);
    throw SystemErrorException(
        __FILENAME__, __LINE__, __func__, std::move(info));
  }
}

#endif
