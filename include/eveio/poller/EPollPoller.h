#pragma once

#include "eveio/PollerBase.h"

#include <sys/epoll.h>
#include <vector>

namespace eveio {

class EPollPoller : public PollerBase<EPollPoller> {
public:
    EPollPoller();
    ~EPollPoller();

    void Poll(std::chrono::milliseconds timeout);
    void UpdateListener(Listener &listener);
    void UnregistListener(Listener &listener);

private:
    void Update(int op, Listener &listener);
    void HandleEvents(int num_events);

    int                               m_epfd;
    std::vector<struct ::epoll_event> m_events;
};

} // namespace eveio
