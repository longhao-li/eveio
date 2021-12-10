#include "eveio/net/Socket.hpp"

#include <spdlog/spdlog.h>

#include <cerrno>
#include <cstring>

using namespace eveio;
using namespace eveio::net;

eveio::net::SocketBase::SocketBase(SocketBase &&other) noexcept
    : sock(other.sock), local_addr(other.local_addr) {
  other.sock = INVALID_NATIVE_SOCKET;
}

SocketBase &eveio::net::SocketBase::operator=(SocketBase &&other) noexcept {
  if (sock != INVALID_NATIVE_SOCKET)
    socket_close(sock);

  sock = other.sock;
  local_addr = other.local_addr;
  other.sock = INVALID_NATIVE_SOCKET;
  return (*this);
}

eveio::net::SocketBase::~SocketBase() noexcept {
  if (sock != INVALID_NATIVE_SOCKET)
    socket_close(sock);
}

bool eveio::net::SocketBase::SetNonblock(bool on) const noexcept {
  if (!socket_set_nonblock(sock, on)) {
    SPDLOG_ERROR("failed to set socket nonblock for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}

bool eveio::net::SocketBase::SetReuseAddr(bool on) const noexcept {
  if (!socket_set_reuseaddr(sock, on)) {
    SPDLOG_ERROR("failed to set socket reuse addr for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}

bool eveio::net::SocketBase::SetReusePort(bool on) const noexcept {
  if (!socket_set_reuseport(sock, on)) {
    SPDLOG_ERROR("failed to set socket reuse port for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}
