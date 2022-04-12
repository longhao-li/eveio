#include "eveio/TcpSocket.h"

using namespace eveio;

eveio::TcpConnection::TcpConnection(const InetAddr &peer) noexcept
    : m_socket(socket::create(peer.GetFamily(), SOCK_STREAM, IPPROTO_TCP)) {
    if (m_socket == INVALID_SOCKET)
        return;

    if (!socket::connect(m_socket, peer.AsSockaddr(), peer.GetAddrSize())) {
        socket::close(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

TcpConnection &eveio::TcpConnection::operator=(TcpConnection &&other) noexcept {
    if (m_socket != INVALID_SOCKET)
        socket::close(m_socket);

    m_socket       = other.m_socket;
    other.m_socket = INVALID_SOCKET;

    return (*this);
}

eveio::TcpSocket::TcpSocket(const InetAddr &addr) noexcept
    : m_socket(socket::create(AF_INET, SOCK_STREAM, 0)) {
    if (!socket::bind(m_socket, addr.AsSockaddr(), addr.GetAddrSize())) {
        socket::close(m_socket);
    }
}

TcpConnection eveio::TcpSocket::Accept() noexcept {
    struct sockaddr_in6 addr;
    size_t              addr_size = sizeof(addr);
    return TcpConnection{socket::accept(
        m_socket, reinterpret_cast<struct sockaddr *>(&addr), &addr_size)};
}
