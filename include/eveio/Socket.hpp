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

#ifndef EVEIO_SOCKET_HPP
#define EVEIO_SOCKET_HPP

#include "eveio/InetAddress.hpp"

#include <cstddef>

namespace eveio {

using socket_t = int;
constexpr const socket_t INVALID_SOCKET = -1;

namespace socket {

socket_t create(int domain, int type, int proto) noexcept;
bool bind(socket_t sock, const struct sockaddr *addr, size_t len) noexcept;
bool listen(socket_t sock, int n) noexcept;
socket_t accept(socket_t sock, struct sockaddr *addr, size_t *len) noexcept;
bool connect(socket_t sock, const struct sockaddr *addr, size_t len) noexcept;
bool close(socket_t sock) noexcept;
bool setnonblock(socket_t sock, bool on) noexcept;
bool settcpnodelay(socket_t sock, bool on) noexcept;
bool setreuseaddr(socket_t sock, bool on) noexcept;
bool setreuseport(socket_t sock, bool on) noexcept;
bool setkeepalive(socket_t sock, bool on) noexcept;
bool shutdown(socket_t sock) noexcept;

int64_t read(socket_t sock, void *buffer, size_t cap) noexcept;
int64_t write(socket_t sock, const void *data, size_t size) noexcept;
int64_t recvfrom(socket_t sock,
                 void *buffer,
                 size_t cap,
                 struct sockaddr *addr,
                 size_t *len) noexcept;
int64_t sendto(socket_t sock,
               const void *buf,
               size_t size,
               const struct sockaddr *addr,
               size_t addrLen) noexcept;

} // namespace socket

class TcpConnection {
  socket_t sock;

public:
  TcpConnection(socket_t connfd) noexcept : sock(connfd) {}

  /// Throw SystemErrorException if failed to connect to peer.
  TcpConnection(const InetAddress &peer);
  ~TcpConnection() noexcept;

  TcpConnection(const TcpConnection &) = delete;
  TcpConnection &operator=(const TcpConnection &) = delete;

  TcpConnection(TcpConnection &&) noexcept;
  TcpConnection &operator=(TcpConnection &&) noexcept;

  /// Throw SystemErrorException if failed.
  InetAddress GetPeerAddress() const;

  int64_t Send(const void *data, size_t size) noexcept;
  int64_t Send(StringRef data) noexcept;
  int64_t Send(const String &data) noexcept;

  int64_t Receive(void *buf, size_t cap) noexcept;

  bool ShutdownWrite() const noexcept;
  bool IsClosed() const noexcept;

  bool SetNoDelay(bool on) const noexcept;
  bool SetNonblock(bool on) const noexcept;
  bool SetKeepAlive(bool on) const noexcept;

  socket_t GetSocket() const noexcept { return sock; }
};

class TcpSocket {
  socket_t sock;

public:
  /// Throw SystemErrorException if failed to bind address.
  TcpSocket(const InetAddress &addr);

  TcpSocket(const TcpSocket &) = delete;
  TcpSocket &operator=(const TcpSocket &) = delete;

  TcpSocket(TcpSocket &&) noexcept;
  TcpSocket &operator=(TcpSocket &&) noexcept;

  ~TcpSocket() noexcept;

  bool Listen(int n) const noexcept;
  TcpConnection Accept() const noexcept;
  bool ShutdownWrite() const noexcept;

  /// Throw SystemErrorException if failed.
  InetAddress GetLocalAddress() const;

  bool SetNonblock(bool on) const noexcept;
  bool SetReuseAddr(bool on) const noexcept;
  bool SetReusePort(bool on) const noexcept;
  bool SetNoDelay(bool on) const noexcept;

  socket_t GetSocket() const noexcept { return sock; }
};

} // namespace eveio

#endif // EVEIO_SOCKET_HPP
