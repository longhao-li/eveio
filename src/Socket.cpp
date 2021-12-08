#include "eveio/net/Socket.hpp"

#include <spdlog/spdlog.h>

#include <cerrno>
#include <cstring>

using namespace eveio;
using namespace eveio::net;
using namespace eveio::net::detail;

eveio::net::detail::SocketBase::SocketBase(detail::SocketBase &&other) noexcept
    : sock(other.sock), local_addr(other.local_addr) {
  other.sock = InvalidSocket;
}

SocketBase &
eveio::net::detail::SocketBase::operator=(SocketBase &&other) noexcept {
  if (sock != InvalidSocket)
    detail::close_socket(sock);

  sock = other.sock;
  local_addr = other.local_addr;
  other.sock = InvalidSocket;
  return (*this);
}

eveio::net::detail::SocketBase::~SocketBase() noexcept {
  if (sock != InvalidSocket)
    detail::close_socket(sock);
}

bool eveio::net::detail::SocketBase::SetNonblock(bool on) const noexcept {
  if (!detail::set_nonblock(sock, on)) {
    SPDLOG_ERROR("failed to set socket nonblock for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}

bool eveio::net::detail::SocketBase::SetReuseAddr(bool on) const noexcept {
  if (!detail::set_reuseaddr(sock, on)) {
    SPDLOG_ERROR("failed to set socket reuse addr for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}

bool eveio::net::detail::SocketBase::SetReusePort(bool on) const noexcept {
  if (!detail::set_reuseport(sock, on)) {
    SPDLOG_ERROR("failed to set socket reuse port for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}
