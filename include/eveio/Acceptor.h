#pragma once

#include "eveio/EventLoop.h"
#include "eveio/Listener.h"
#include "eveio/TcpSocket.h"

#include <cassert>
#include <functional>

namespace eveio {

class Acceptor {
public:
    using NewConnectionCallback = std::function<void(TcpConnection &&)>;

    Acceptor(EventLoop &loop, const InetAddr &local_addr);
    ~Acceptor();

    Acceptor(const Acceptor &) = delete;
    Acceptor &operator=(const Acceptor &) = delete;

    Acceptor(Acceptor &&) = delete;
    Acceptor &operator=(Acceptor &&) = delete;

    void SetNewConnectionCallback(NewConnectionCallback cb) {
        m_new_conn_callback = std::move(cb);
    }

    bool Listen() noexcept;

    bool IsListening() const noexcept { return m_is_listening; }

    bool GetLocalAddr(InetAddr &addr) const noexcept {
        return m_socket.GetLocalAddr(addr);
    }

    void Quit() {
        m_listener.DisableAll();
        m_listener.Unregister();
        m_is_listening = false;
    }

private:
    EventLoop *const      m_loop;
    bool                  m_is_listening;
    TcpSocket             m_socket;
    Listener              m_listener;
    NewConnectionCallback m_new_conn_callback;
};

} // namespace eveio
