#include "eveio/net/TcpSocket.hpp"

#include "eveio/Result.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/TcpConnection.hpp"

#include <spdlog/spdlog.h>

#include <cerrno>
#include <cstring>

using eveio::Result;
using eveio::net::TcpConnection;
using eveio::net::TcpSocket;

Result<TcpSocket> eveio::net::TcpSocket::Create(const InetAddr &addr) noexcept {
  native_socket_type sock = detail::socket(addr.GetFamily(), SOCK_STREAM, 0);
  if (sock == InvalidSocket)
    return Result<TcpSocket>::Error(std::strerror(errno));

  if (!detail::bind(sock, addr.AsSockaddr(), addr.Size())) {
    int saved_errno = errno;
    detail::close_socket(sock);
    return Result<TcpSocket>::Error(std::strerror(saved_errno));
  }

  return Result<TcpSocket>::Ok(TcpSocket(sock, addr));
}

eveio::net::TcpSocket::TcpSocket(TcpSocket &&other) noexcept
    : sock(other.sock), local_addr(other.local_addr) {
  other.sock = InvalidSocket;
}

eveio::net::TcpSocket &
eveio::net::TcpSocket::operator=(TcpSocket &&other) noexcept {
  if (sock != InvalidSocket)
    detail::close_socket(sock);

  sock = other.sock;
  local_addr = other.local_addr;
  other.sock = InvalidSocket;
  return (*this);
}

eveio::net::TcpSocket::~TcpSocket() noexcept {
  if (sock != InvalidSocket)
    detail::close_socket(sock);
}

bool eveio::net::TcpSocket::Listen(int n) const noexcept {
  return detail::listen(sock, n);
}

void eveio::net::TcpSocket::CloseWrite() const noexcept {
  if (!detail::close_tcp_write(sock))
    SPDLOG_ERROR("failed to close tcp write for socket {}: {}.",
                 sock,
                 std::strerror(errno));
}

void eveio::net::TcpSocket::SetReuseAddr(bool on) const noexcept {
  if (!detail::set_reuseaddr(sock, on))
    SPDLOG_ERROR("failed to set socket reuse addr for {}: {}.",
                 sock,
                 std::strerror(errno));
}

void eveio::net::TcpSocket::SetReusePort(bool on) const noexcept {
  if (!detail::set_reuseport(sock, on))
    SPDLOG_ERROR("failed to set socket reuse port for {}: {}.",
                 sock,
                 std::strerror(errno));
}

Result<TcpConnection> eveio::net::TcpSocket::Accept() const noexcept {
  struct sockaddr_in6 addr {};
  socklen_t len;

  native_socket_type conn_sock =
      detail::accept(sock, reinterpret_cast<struct sockaddr *>(&addr), &len);
  if (conn_sock == InvalidSocket)
    return Result<TcpConnection>::Error(std::strerror(errno));

  return Result<TcpConnection>::Ok(TcpConnection(conn_sock, addr));
}
