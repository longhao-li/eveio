#pragma once

#include "eveio/Acceptor.h"
#include "eveio/AsyncTcpConnection.h"
#include "eveio/EventLoopThreadPool.h"

#include <memory>

namespace eveio {

class TcpServer {
public:
    TcpServer(EventLoop &loop, const InetAddr &listen_addr);
    TcpServer(EventLoop &loop, const InetAddr &listen_addr,
              std::shared_ptr<EventLoopThreadPool> pool);
    ~TcpServer();

    TcpServer(const TcpServer &) = delete;
    TcpServer &operator=(const TcpServer &) = delete;

    TcpServer(TcpServer &&) = delete;
    TcpServer &operator=(TcpServer &&) = delete;

    bool GetLocalAddr(InetAddr &addr) const noexcept {
        return m_acceptor->GetLocalAddr(addr);
    }

    std::shared_ptr<EventLoopThreadPool>
    GetEventLoopThreadPool() const noexcept {
        return m_pool;
    }

    EventLoop &GetAcceptorPool() const noexcept { return *m_loop; }

    void SetConnectionCallback(TcpConnectionCallback cb) noexcept {
        m_conn_callback = std::move(cb);
    }

    void SetMessageCallback(TcpMessageCallback cb) noexcept {
        m_msg_callback = std::move(cb);
    }

    void SetWriteCompleteCallback(TcpWriteCompleteCallback cb) noexcept {
        m_write_complete_callback = std::move(cb);
    }

    void Start();

private:
    EventLoop *const                     m_loop;
    std::shared_ptr<EventLoopThreadPool> m_pool;
    std::shared_ptr<Acceptor>            m_acceptor;

    std::atomic_bool m_is_started;

    TcpConnectionCallback    m_conn_callback;
    TcpMessageCallback       m_msg_callback;
    TcpWriteCompleteCallback m_write_complete_callback;
};

} // namespace eveio
