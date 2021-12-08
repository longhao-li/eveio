#include "eveio/net/TcpSocket.hpp"

#include "eveio/Result.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/TcpConnection.hpp"

#include <spdlog/spdlog.h>

#include <cerrno>
#include <cstring>

using namespace eveio;
using namespace eveio::net;

/// Muduo - A reactor-based C++ network library for Linux
/// Copyright (c) 2010, Shuo Chen.  All rights reserved.
/// http://code.google.com/p/muduo/
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions
/// are met:
///
///   * Redistributions of source code must retain the above copyright
/// notice, this list of conditions and the following disclaimer.
///   * Redistributions in binary form must reproduce the above copyright
/// notice, this list of conditions and the following disclaimer in the
/// documentation and/or other materials provided with the distribution.
///   * Neither the name of Shuo Chen nor the names of other contributors
/// may be used to endorse or promote products derived from this software
/// without specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
/// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
/// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
/// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
/// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

TcpSocket &eveio::net::TcpSocket::operator=(TcpSocket &&other) noexcept {
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

bool eveio::net::TcpSocket::SetNonblock(bool on) const noexcept {
  if (!detail::set_nonblock(sock, on)) {
    SPDLOG_ERROR("failed to set socket nonblock for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}

bool eveio::net::TcpSocket::SetReuseAddr(bool on) const noexcept {
  if (!detail::set_reuseaddr(sock, on)) {
    SPDLOG_ERROR("failed to set socket reuse addr for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}

bool eveio::net::TcpSocket::SetReusePort(bool on) const noexcept {
  if (!detail::set_reuseport(sock, on)) {
    SPDLOG_ERROR("failed to set socket reuse port for {}: {}.",
                 sock,
                 std::strerror(errno));
    return false;
  }
  return true;
}

Result<TcpConnection> eveio::net::TcpSocket::Accept() const noexcept {
  struct sockaddr_in6 addr {};
  socklen_t len = sizeof(addr);

  native_socket_type conn_sock =
      detail::accept(sock, reinterpret_cast<struct sockaddr *>(&addr), &len);
  if (conn_sock == InvalidSocket)
    return Result<TcpConnection>::Error(std::strerror(errno));

  return Result<TcpConnection>::Ok(TcpConnection(conn_sock, addr));
}
