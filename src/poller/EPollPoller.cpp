#include "eveio/poller/EPollPoller.h"
#include "eveio/Listener.h"

#include <cassert>
#include <unistd.h>

using namespace eveio;

static constexpr const size_t DEFAULT_EVENTLIST_SIZE = 16;

enum {
    POLLER_STATE_INIT  = 0,
    POLLER_STATE_ADDED = 1,
};

static uint32_t MapEvent(uint32_t ep_event) noexcept {
    uint32_t e = 0;
    if (ep_event & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        e |= EVENT_READ;
    }

    if (ep_event & (EPOLLOUT)) {
        e |= EVENT_WRITE;
    }

    return e;
}

static uint32_t UnmapEvent(uint32_t e) noexcept {
    uint32_t res = 0;
    if (e & EVENT_READ) {
        res |= (EPOLLIN | EPOLLPRI | EPOLLRDHUP);
    }

    if (e & EVENT_WRITE) {
        res |= (EPOLLOUT);
    }
    return res;
}

eveio::EPollPoller::EPollPoller()
    : m_epfd(::epoll_create1(EPOLL_CLOEXEC)), m_events(DEFAULT_EVENTLIST_SIZE) {
    assert(m_epfd >= 0);
}

eveio::EPollPoller::~EPollPoller() { ::close(m_epfd); }

void eveio::EPollPoller::UpdateListener(Listener &listener) {
    if (listener.GetPollerState() == POLLER_STATE_INIT) {
        if (!listener.IsNoneEvent()) {
            listener.SetPollerState(POLLER_STATE_ADDED);
            Update(EPOLL_CTL_ADD, listener);
        }
    } else {
        if (listener.IsNoneEvent())
            Update(EPOLL_CTL_DEL, listener);
        else
            Update(EPOLL_CTL_MOD, listener);
    }
}

void eveio::EPollPoller::UnregistListener(Listener &listener) {
    if (listener.GetPollerState() == POLLER_STATE_ADDED) {
        Update(EPOLL_CTL_DEL, listener);
        listener.SetPollerState(POLLER_STATE_INIT);
    }
}

void eveio::EPollPoller::Update(int op, Listener &listener) {
    struct ::epoll_event event {};
    event.events   = UnmapEvent(listener.EventsListening());
    event.data.ptr = &listener;

    ::epoll_ctl(m_epfd, op, listener.GetFD(), &event);
}

void eveio::EPollPoller::Poll(std::chrono::milliseconds timeout) {
    int num_events = ::epoll_wait(m_epfd,
                                  m_events.data(),
                                  static_cast<int>(m_events.capacity()),
                                  static_cast<int>(timeout.count()));
    if (num_events > 0) {
        HandleEvents(num_events);
        if (num_events == static_cast<int>(m_events.capacity())) {
            m_events.reserve(m_events.capacity() * 2);
        }
    }
}

void eveio::EPollPoller::HandleEvents(int num_events) {
    for (size_t i = 0; i < static_cast<size_t>(num_events); ++i) {
        struct ::epoll_event &event = m_events[i];
        auto listener               = static_cast<Listener *>(event.data.ptr);
        assert(listener != nullptr);
        uint32_t e = MapEvent(event.events);

        if (e & EVENT_READ) {
            if (listener->GetReadCallback())
                listener->GetReadCallback()(listener);
        }

        if (e & EVENT_WRITE) {
            if (listener->GetWriteCallback())
                listener->GetWriteCallback()(listener);
        }
    }
}
