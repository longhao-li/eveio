#include "eveio/net/TcpConnection.hpp"
#include "eveio/net/Socket.hpp"

#include <spdlog/spdlog.h>

using eveio::Result;
using eveio::net::TcpConnection;

Result<TcpConnection>
eveio::net::TcpConnection::Connect(const InetAddr &peer) noexcept {
  native_socket_type conn_sock =
      detail::socket(peer.GetFamily(), SOCK_STREAM, 0);
  if (conn_sock == InvalidSocket)
    return Result<TcpConnection>::Error(std::strerror(errno));

  struct sockaddr_in6 addr;
  if (!detail::connect(
          conn_sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)))
    return Result<TcpConnection>::Error(std::strerror(errno));

  return Result<TcpConnection>::Ok(TcpConnection(conn_sock, addr));
}

eveio::net::TcpConnection::TcpConnection(TcpConnection &&other) noexcept
    : conn_handle(other.conn_handle),
      create_time(other.create_time),
      peer_addr(other.peer_addr) {
  other.conn_handle = InvalidSocket;
}

TcpConnection &
eveio::net::TcpConnection::operator=(TcpConnection &&other) noexcept {
  if (conn_handle != InvalidSocket)
    detail::close_socket(conn_handle);
  conn_handle = other.conn_handle;
  create_time = other.create_time;
  peer_addr = other.peer_addr;
  other.conn_handle = InvalidSocket;
  return (*this);
}

eveio::net::TcpConnection::~TcpConnection() noexcept {
  if (conn_handle != InvalidSocket)
    detail::close_socket(conn_handle);
}

int eveio::net::TcpConnection::Send(StringRef data) const noexcept {
  return detail::socket_write(conn_handle, data.data(), data.size());
}

int eveio::net::TcpConnection::Send(const void *buf,
                                    size_t byte) const noexcept {
  return detail::socket_write(conn_handle, buf, byte);
}

int eveio::net::TcpConnection::Receive(void *buf, size_t cap) const noexcept {
  return detail::socket_read(conn_handle, buf, cap);
}

void eveio::net::TcpConnection::CloseWrite() const noexcept {
  if (!detail::close_tcp_write(conn_handle))
    SPDLOG_ERROR("failed to close write for socket {}: {}.",
                 conn_handle,
                 std::strerror(errno));
}

bool eveio::net::TcpConnection::IsClosed() const noexcept {
  struct sockaddr_in6 addr;
  socklen_t len = peer_addr.Size();
  int res = ::getpeername(
      conn_handle, reinterpret_cast<struct sockaddr *>(&addr), &len);
  return (res < 0 && errno == ENOTCONN);
}

bool eveio::net::TcpConnection::SetNoDelay(bool on) const noexcept {
  if (!detail::set_tcp_nodelay(conn_handle, on)) {
    SPDLOG_ERROR("failed to set tcp nodelay for socket {}: {}.",
                 conn_handle,
                 std::strerror(errno));
    return false;
  }
  return true;
}

bool eveio::net::TcpConnection::SetNonblock(bool on) const noexcept {
  if (!detail::set_nonblock(conn_handle, on)) {
    SPDLOG_ERROR("failed to set connection non block for socket {}: {}.",
                 conn_handle,
                 std::strerror(errno));
    return false;
  }
  return true;
}

bool eveio::net::TcpConnection::SetKeepAlive(bool on) const noexcept {
  if (!detail::set_keepalive(conn_handle, on)) {
    SPDLOG_ERROR("failed to set connection keep alive for socket {}: {}.",
                 conn_handle,
                 std::strerror(errno));
    return false;
  }
  return true;
}
