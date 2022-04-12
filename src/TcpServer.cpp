#include "eveio/TcpServer.h"

#include <cstdio>
#include <cstdlib>

using namespace eveio;

eveio::TcpServer::TcpServer(EventLoop &loop, const InetAddr &listen_addr)
    : TcpServer(loop, listen_addr, std::make_shared<EventLoopThreadPool>()) {}

eveio::TcpServer::TcpServer(EventLoop &loop, const InetAddr &listen_addr,
                            std::shared_ptr<EventLoopThreadPool> pool)
    : m_loop(&loop),
      m_pool(std::move(pool)),
      m_acceptor(std::make_shared<Acceptor>(*m_loop, listen_addr)),
      m_is_started(false),
      m_conn_callback(),
      m_msg_callback(),
      m_write_complete_callback() {
    m_acceptor->SetNewConnectionCallback([this](TcpConnection &&conn) {
        conn.SetNonBlock(true);
        conn.SetKeepAlive(true);
        EventLoop *worker = this->m_pool->GetNextLoop();
        auto async_conn   = new AsyncTcpConnection(*worker, std::move(conn));

        if (m_msg_callback)
            async_conn->SetMessageCallback(m_msg_callback);

        if (m_write_complete_callback)
            async_conn->SetWriteCompleteCallback(m_write_complete_callback);

        if (m_conn_callback)
            m_conn_callback(async_conn);
    });
}

eveio::TcpServer::~TcpServer() {
    {
        std::shared_ptr<Acceptor> guard = m_acceptor;
        m_loop->RunInLoop([guard]() { guard->Quit(); });
    }
}

void eveio::TcpServer::Start() {
    if (m_is_started.exchange(true, std::memory_order_relaxed))
        return;

    m_loop->RunInLoop([this]() {
        if (!m_acceptor->Listen()) {
            fprintf(stderr,
                    "eveio::TcpServer::Start - Acceptor failed to listen.\n");
            std::abort();
        }
    });
    m_pool->Start();
}
