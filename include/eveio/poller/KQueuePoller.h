#pragma once

#include "eveio/PollerBase.h"

#include <sys/event.h>

namespace eveio {

class KQueuePoller final : public PollerBase<KQueuePoller> {
public:
    KQueuePoller();
    ~KQueuePoller();

    void Poll(std::chrono::milliseconds timeout);
    void UpdateListener(Listener &listener);
    void UnregistListener(Listener &listener);

private:
    void HandleEvents(int num_events);
    void Update(uint32_t rw, uint16_t ext_flags, Listener &listener);

    int                          m_kq_fd;
    std::vector<struct ::kevent> m_events;
};

} // namespace eveio
