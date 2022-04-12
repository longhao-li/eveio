#include "eveio/poller/KQueuePoller.h"
#include "eveio/Listener.h"

#include <cassert>
#include <cerrno>
#include <unistd.h>

using namespace eveio;

static constexpr const size_t DEFAULT_EVENTLIST_SIZE = 16;

enum {
    POLLER_STATE_INIT  = 0,
    POLLER_STATE_ADDED = 1,
};

eveio::KQueuePoller::KQueuePoller()
    : m_kq_fd(::kqueue()), m_events(DEFAULT_EVENTLIST_SIZE) {
    assert(m_kq_fd >= 0);
}

eveio::KQueuePoller::~KQueuePoller() { ::close(m_kq_fd); }

void eveio::KQueuePoller::UpdateListener(Listener &listener) {
    uint16_t ext_flags = 0;
    if (listener.GetPollerState() == POLLER_STATE_INIT) {
        ext_flags |= EV_ADD;
        listener.SetPollerState(POLLER_STATE_ADDED);
    }

    Update(EVENT_READ, ext_flags, listener);
    Update(EVENT_WRITE, ext_flags, listener);
}

void eveio::KQueuePoller::UnregistListener(Listener &listener) {
    if (listener.GetPollerState() != POLLER_STATE_INIT) {
        Update(EVENT_READ, EV_DELETE, listener);
        Update(EVENT_WRITE, EV_DELETE, listener);
        listener.SetPollerState(POLLER_STATE_INIT);
    }
}

void eveio::KQueuePoller::Update(uint32_t rw, uint16_t ext_flags,
                                 Listener &listener) {
    int16_t  rw_flag = (rw == EVENT_READ) ? EVFILT_READ : EVFILT_WRITE;
    uint16_t flag = (listener.EventsListening() & rw) ? EV_ENABLE : EV_DISABLE;

    struct ::kevent change;
    EV_SET(
        &change, listener.GetFD(), rw_flag, flag | ext_flags, 0, 0, &listener);

    struct ::timespec timeout {};
    ::kevent(m_kq_fd, &change, 1, nullptr, 0, &timeout);
}

void eveio::KQueuePoller::Poll(std::chrono::milliseconds timeout) {
    struct ::timespec poll_timeout {
        timeout.count() / 1000, (timeout.count() % 1000) * 1000 * 1000
    };

    int num_events = ::kevent(m_kq_fd,
                              nullptr,
                              0,
                              m_events.data(),
                              static_cast<int>(m_events.capacity()),
                              &poll_timeout);

    // Handle events.
    if (num_events > 0) {
        HandleEvents(num_events);

        if (num_events == static_cast<int>(m_events.capacity())) {
            m_events.reserve(m_events.capacity() * 2);
        }
    }
}

void eveio::KQueuePoller::HandleEvents(int num_events) {
    for (size_t i = 0; i < static_cast<size_t>(num_events); ++i) {
        struct ::kevent &event    = m_events[i];
        auto             listener = static_cast<Listener *>(event.udata);
        assert(listener != nullptr);

        if (event.filter == EVFILT_READ) {
            if (listener->IsReading() && listener->GetReadCallback()) {
                listener->GetReadCallback()(listener);
            }
        } else if (event.filter == EVFILT_WRITE) {
            if (listener->IsWriting() && listener->GetWriteCallback()) {
                listener->GetWriteCallback()(listener);
            }
        }
    }
}
