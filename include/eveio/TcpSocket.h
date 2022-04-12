#pragma once

#include "eveio/Socket.h"

// #include <cstdio>

namespace eveio {

class TcpConnection {
public:
    TcpConnection() noexcept = default;

    explicit TcpConnection(const InetAddr &peer) noexcept;
    explicit TcpConnection(socket_t conn_sock) noexcept : m_socket(conn_sock) {}

    TcpConnection(const TcpConnection &) = delete;
    TcpConnection &operator=(const TcpConnection &) = delete;

    TcpConnection(TcpConnection &&other) noexcept : m_socket(other.m_socket) {
        other.m_socket = INVALID_SOCKET;

        // printf("TcpConnection: other.m_socket: %d, this->m_socket: %d\n",
        //        other.m_socket,
        //        m_socket);
    }

    TcpConnection &operator=(TcpConnection &&other) noexcept;

    ~TcpConnection() {
        if (m_socket != INVALID_SOCKET)
            socket::close(m_socket);
    }

    bool IsValid() const noexcept { return m_socket != INVALID_SOCKET; }

    bool GetPeerAddr(InetAddr &addr) const noexcept {
        return socket::getpeername(m_socket, addr);
    }

    int64_t Send(const void *data, size_t size) noexcept {
        return socket::write(m_socket, data, size);
    }

    int64_t Receive(void *buffer, size_t size) noexcept {
        return socket::read(m_socket, buffer, size);
    }

    bool ShutdownWrite() noexcept { return socket::shutdown_write(m_socket); }

    bool SetNoDelay(bool on) noexcept {
        return socket::settcpnodelay(m_socket, on);
    }

    bool SetNonBlock(bool on) noexcept {
        return socket::setnonblock(m_socket, on);
    }

    bool SetKeepAlive(bool on) noexcept {
        return socket::setkeepalive(m_socket, on);
    }

    socket_t GetSocket() const noexcept { return m_socket; }

private:
    socket_t m_socket = INVALID_SOCKET;
};

class TcpSocket {
public:
    TcpSocket() noexcept = default;
    TcpSocket(const InetAddr &addr) noexcept;

    TcpSocket(const TcpSocket &) = delete;
    TcpSocket &operator=(const TcpSocket &) = delete;

    TcpSocket(TcpSocket &&) noexcept;
    TcpSocket &operator=(TcpSocket &&) noexcept;

    ~TcpSocket() {
        if (m_socket != INVALID_SOCKET)
            socket::close(m_socket);
    }

    bool Listen(int n = SOMAXCONN) noexcept {
        return socket::listen(m_socket, n);
    }

    TcpConnection Accept() noexcept;

    bool GetLocalAddr(InetAddr &addr) const noexcept {
        return socket::getpeername(m_socket, addr);
    }

    bool SetNonBlock(bool on) noexcept {
        return socket::setnonblock(m_socket, on);
    }

    bool SetReuseAddr(bool on) noexcept {
        return socket::setreuseaddr(m_socket, on);
    }

    bool SetReusePort(bool on) noexcept {
        return socket::setreuseport(m_socket, on);
    }

    bool IsValid() const noexcept { return m_socket != INVALID_SOCKET; }

    socket_t GetSocket() const noexcept { return m_socket; }

private:
    socket_t m_socket = INVALID_SOCKET;
};

} // namespace eveio
