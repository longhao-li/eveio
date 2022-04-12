#pragma once

#include "eveio/EventLoop.h"

#include <cstddef>
#include <cstdint>

namespace eveio {

enum {
    EVENT_NONE  = 0x00,
    EVENT_READ  = 0x01,
    EVENT_WRITE = 0x02,
};

class EventLoop;

class Listener {
public:
    using Callback = auto (*)(Listener *) -> void;

    Listener(EventLoop &loop, int fd) noexcept : m_loop(&loop), m_fd(fd) {}
    Listener(EventLoop &loop, void *handle) noexcept
        : m_loop(&loop), m_handle(handle) {}

    Listener(const Listener &) = delete;
    Listener &operator=(const Listener &) = delete;

    ~Listener() { Unregister(); }

    void  TieObject(void *object) noexcept { m_tied_object = object; }
    void *GetTiedObject() const noexcept { return m_tied_object; }

    void SetReadCallback(Callback cb) noexcept { m_read_callback = cb; }
    void SetWriteCallback(Callback cb) noexcept { m_write_callback = cb; }

    Callback GetReadCallback() const noexcept { return m_read_callback; }
    Callback GetWriteCallback() const noexcept { return m_read_callback; }

    EventLoop &GetLoop() const noexcept { return *m_loop; }
    int        GetFD() const noexcept { return m_fd; }
    void *     GetHandle() const noexcept { return m_handle; }

    void EnableReading() noexcept {
        m_events_listening |= EVENT_READ;
        m_loop->UpdateListener(*this);
    }

    void EnableWriting() noexcept {
        m_events_listening |= EVENT_WRITE;
        m_loop->UpdateListener(*this);
    }

    void DisableReading() noexcept {
        m_events_listening &= (~EVENT_READ);
        m_loop->UpdateListener(*this);
    }

    void DisableWriting() noexcept {
        m_events_listening &= (~EVENT_WRITE);
        m_loop->UpdateListener(*this);
    }

    void DisableAll() noexcept {
        m_events_listening = EVENT_NONE;
        m_loop->UpdateListener(*this);
    }

    void Unregister() noexcept { m_loop->UnregistListener(*this); }

    uint32_t EventsListening() const noexcept { return m_events_listening; }

    uint32_t GetPollerState() const noexcept { return m_poller_state; }
    void     SetPollerState(uint32_t state) noexcept { m_poller_state = state; }

    bool IsReading() const noexcept {
        return (m_events_listening & EVENT_READ);
    }

    bool IsWriting() const noexcept {
        return (m_events_listening & EVENT_WRITE);
    }

    bool IsNoneEvent() const noexcept {
        return m_events_listening == EVENT_NONE;
    }

private:
    EventLoop *m_loop = nullptr;
    union {
        int   m_fd;
        void *m_handle;
    };
    void *   m_tied_object      = nullptr;
    uint32_t m_poller_state     = 0;
    uint32_t m_events_listening = EVENT_NONE;
    Callback m_read_callback    = nullptr;
    Callback m_write_callback   = nullptr;
};

} // namespace eveio
