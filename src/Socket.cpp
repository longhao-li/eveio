/// Copyright (c) 2021 Li Longhao
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.

#include "eveio/Socket.hpp"
#include "eveio/Config.hpp"
#include "eveio/Exception.hpp"

#include <cstring>

using namespace eveio;

#if EVEIO_OS_WIN32
#else
socket_t eveio::socket::create(int domain, int type, int proto) noexcept {
  socket_t sock = ::socket(domain, type, proto);
  if (sock < 0) {
    return INVALID_SOCKET;
  } else {
    return sock;
  }
}

bool eveio::socket::bind(socket_t sock,
                         const struct sockaddr *addr,
                         size_t len) noexcept {
  if (::bind(sock, addr, static_cast<socklen_t>(len)) < 0) {
    return false;
  }
  return true;
}

bool eveio::socket::listen(socket_t sock, int n) noexcept {
  return (::listen(sock, n) >= 0);
}

socket_t eveio::socket::accept(socket_t sock,
                               struct sockaddr *addr,
                               size_t *len) noexcept {
  auto tempLen = static_cast<socklen_t>(*len);
  socket_t res = ::accept(sock, addr, &tempLen);
  if (res < 0) {
    return INVALID_SOCKET;
  }

  *len = static_cast<size_t>(tempLen);
  return res;
}

bool eveio::socket::connect(socket_t sock,
                            const struct sockaddr *addr,
                            size_t len) noexcept {
  return (::connect(sock, addr, static_cast<socklen_t>(len)) >= 0);
}

bool eveio::socket::close(socket_t sock) noexcept {
  return (::close(sock) == 0);
}

bool eveio::socket::setnonblock(socket_t sock, bool on) noexcept {
  int opt = ::fcntl(sock, F_GETFL);
  if (on) {
    opt |= O_NONBLOCK;
  } else {
    opt &= ~O_NONBLOCK;
  }
  return (::fcntl(sock, F_SETFL, opt) >= 0);
}

bool eveio::socket::settcpnodelay(socket_t sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) >= 0);
}

bool eveio::socket::setreuseaddr(socket_t sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) >= 0);
}

bool eveio::socket::setreuseport(socket_t sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) >= 0);
}

bool eveio::socket::setkeepalive(socket_t sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) >= 0);
}

bool eveio::socket::shutdown(socket_t sock) noexcept {
  return (::shutdown(sock, SHUT_WR) >= 0);
}

int64_t eveio::socket::read(socket_t sock, void *buffer, size_t cap) noexcept {
  return ::recv(sock, buffer, cap, 0);
}

int64_t
eveio::socket::write(socket_t sock, const void *data, size_t size) noexcept {
  return ::send(sock, data, size, MSG_NOSIGNAL);
}

int64_t eveio::socket::recvfrom(socket_t sock,
                                void *buffer,
                                size_t cap,
                                struct sockaddr *addr,
                                size_t *len) noexcept {
  auto tempLen = static_cast<socklen_t>(*len);
  auto res = ::recvfrom(sock, buffer, cap, 0, addr, &tempLen);
  if (res < 0) {
    return -1;
  }
  *len = static_cast<size_t>(tempLen);
  return res;
}

int64_t eveio::socket::sendto(socket_t sock,
                              const void *buf,
                              size_t size,
                              const struct sockaddr *addr,
                              size_t addrLen) noexcept {
  return ::sendto(
      sock, buf, size, MSG_NOSIGNAL, addr, static_cast<socklen_t>(addrLen));
}
#endif

eveio::TcpConnection::TcpConnection(const InetAddress &peer)
    : sock(socket::create(peer.GetFamily(), SOCK_STREAM, IPPROTO_TCP)) {
  if (sock == INVALID_SOCKET) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Failed to create socket: ") +
                                   std::strerror(errno));
  }

  if (!socket::connect(sock, peer.AsSockaddr(), peer.size())) {
    socket::close(sock);
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Failed to connect to ") +
                                   peer.GetIPWithPort());
  }
}

eveio::TcpConnection::~TcpConnection() noexcept {
  if (sock != INVALID_SOCKET) {
    socket::close(sock);
  }
}

eveio::TcpConnection::TcpConnection(TcpConnection &&other) noexcept
    : sock(other.sock) {
  other.sock = INVALID_SOCKET;
}

TcpConnection &eveio::TcpConnection::operator=(TcpConnection &&other) noexcept {
  if (this == &other) {
    return (*this);
  }

  if (sock != INVALID_SOCKET) {
    socket::close(sock);
  }

  sock = other.sock;
  other.sock = INVALID_SOCKET;

  return (*this);
}

InetAddress eveio::TcpConnection::GetPeerAddress() const {
  struct sockaddr_in6 addr {};
  socklen_t len = sizeof(addr);
  if (::getpeername(sock, reinterpret_cast<struct sockaddr *>(&addr), &len) <
      0) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Failed to get peer address: ") +
                                   std::strerror(errno));
  }
  return {addr};
}

int64_t eveio::TcpConnection::Send(const void *data, size_t size) noexcept {
  return socket::write(sock, data, size);
}

int64_t eveio::TcpConnection::Send(StringRef data) noexcept {
  return Send(data.data(), data.size());
}

int64_t eveio::TcpConnection::Send(const String &data) noexcept {
  return Send(data.data(), data.size());
}

int64_t eveio::TcpConnection::Receive(void *buf, size_t cap) noexcept {
  return socket::read(sock, buf, cap);
}

bool eveio::TcpConnection::ShutdownWrite() const noexcept {
  return socket::shutdown(sock);
}

bool eveio::TcpConnection::IsClosed() const noexcept {
  struct sockaddr_in6 addr {};
  auto len = static_cast<socklen_t>(sizeof(addr));
  int res =
      ::getpeername(sock, reinterpret_cast<struct sockaddr *>(&addr), &len);
  return (res < 0 && errno == ENOTCONN);
}

bool eveio::TcpConnection::SetNoDelay(bool on) const noexcept {
  return socket::settcpnodelay(sock, on);
}

bool eveio::TcpConnection::SetNonblock(bool on) const noexcept {
  return socket::setnonblock(sock, on);
}

bool eveio::TcpConnection::SetKeepAlive(bool on) const noexcept {
  return socket::setkeepalive(sock, on);
}

eveio::TcpSocket::TcpSocket(const InetAddress &addr)
    : sock(socket::create(AF_INET, SOCK_STREAM, 0)) {
  if (sock == INVALID_SOCKET) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Failed to create socket: ") +
                                   std::strerror(errno));
  }

  if (!socket::bind(sock, addr.AsSockaddr(), addr.size())) {
    socket::close(sock);
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Failed to bind socket: ") +
                                   std::strerror(errno));
  }
}

eveio::TcpSocket::TcpSocket(TcpSocket &&other) noexcept : sock(other.sock) {
  other.sock = INVALID_SOCKET;
}

TcpSocket &eveio::TcpSocket::operator=(TcpSocket &&other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (sock != INVALID_SOCKET) {
    socket::close(sock);
  }

  sock = other.sock;
  other.sock = INVALID_SOCKET;

  return (*this);
}

eveio::TcpSocket::~TcpSocket() noexcept {
  if (sock != INVALID_SOCKET) {
    socket::close(sock);
  }
}

bool eveio::TcpSocket::Listen(int n) const noexcept {
  return socket::listen(sock, n);
}

TcpConnection eveio::TcpSocket::Accept() const noexcept {
  struct sockaddr_in6 addr {};
  size_t addrLen = sizeof(addr);
  auto connSock = socket::accept(
      this->sock, reinterpret_cast<struct sockaddr *>(&addr), &addrLen);
  return {connSock};
}

bool eveio::TcpSocket::ShutdownWrite() const noexcept {
  return socket::shutdown(sock);
}

InetAddress eveio::TcpSocket::GetLocalAddress() const {
  struct sockaddr_in6 addr;
  socklen_t len = sizeof(addr);
  if (::getsockname(sock, reinterpret_cast<struct sockaddr *>(&addr), &len) <
      0) {
    throw SystemErrorException(
        __FILENAME__,
        __LINE__,
        __func__,
        String("Failed to get local address for socket ") +
            std::to_string(sock) + ": " + std::strerror(errno));
  }

  return {addr};
}

bool eveio::TcpSocket::SetNonblock(bool on) const noexcept {
  return socket::setnonblock(sock, on);
}

bool eveio::TcpSocket::SetReuseAddr(bool on) const noexcept {
  return socket::setreuseaddr(sock, on);
}

bool eveio::TcpSocket::SetReusePort(bool on) const noexcept {
  return socket::setreuseport(sock, on);
}

bool eveio::TcpSocket::SetNoDelay(bool on) const noexcept {
  return socket::settcpnodelay(sock, on);
}
