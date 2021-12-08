#ifndef EVEIO_NET_TCPSOCKET_HPP
#define EVEIO_NET_TCPSOCKET_HPP

#include "eveio/Handle.hpp"
#include "eveio/Result.hpp"
#include "eveio/net/Config.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/TcpConnection.hpp"

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

namespace eveio {
namespace net {

class TcpSocket {
public:
  typedef detail::native_socket_type native_socket_type;
  static constexpr native_socket_type InvalidSocket = detail::InvalidSocket;

private:
  native_socket_type sock;
  InetAddr local_addr;

  TcpSocket(native_socket_type h, InetAddr addr) noexcept
      : sock(h), local_addr(addr) {}

public:
  static Result<TcpSocket> Create(const InetAddr &local_addr) noexcept;

  TcpSocket(const TcpSocket &) = delete;
  TcpSocket &operator=(const TcpSocket &) = delete;

  TcpSocket(TcpSocket &&other) noexcept;
  TcpSocket &operator=(TcpSocket &&other) noexcept;

  ~TcpSocket() noexcept;

  bool Listen(int n) const noexcept;
  void CloseWrite() const noexcept;
  bool SetNonblock(bool on) const noexcept;
  bool SetReuseAddr(bool on) const noexcept;
  bool SetReusePort(bool on) const noexcept;

  Result<TcpConnection> Accept() const noexcept;

  const InetAddr &LocalAddr() const noexcept { return local_addr; }

  native_socket_type native_socket() const noexcept { return sock; }

  operator native_socket_type() const noexcept { return sock; }
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_TCPSOCKET_HPP
