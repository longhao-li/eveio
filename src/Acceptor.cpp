#include "eveio/Acceptor.h"

#include <cstdio>

using namespace eveio;

eveio::Acceptor::Acceptor(EventLoop &loop, const InetAddr &local_addr)
    : m_loop(&loop),
      m_is_listening(false),
      m_socket(local_addr),
      m_listener(*m_loop, m_socket.GetSocket()),
      m_new_conn_callback() {
    m_listener.TieObject(this);

    m_listener.SetReadCallback(+[](Listener *listener) {
        auto acceptor      = static_cast<Acceptor *>(listener->GetTiedObject());
        TcpConnection conn = acceptor->m_socket.Accept();

        if (conn.IsValid()) {
            if (acceptor->m_new_conn_callback) {
                acceptor->m_new_conn_callback(std::move(conn));
            }
        }
    });
}

eveio::Acceptor::~Acceptor() = default;

bool eveio::Acceptor::Listen() noexcept {
    if (m_is_listening)
        return true;

    if (!m_socket.Listen()) {
        m_is_listening = false;
        return false;
    } else {
        this->m_listener.EnableReading();
        return true;
    }
}
