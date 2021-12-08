#ifndef EVEIO_NET_ACCEPTOR_HPP
#define EVEIO_NET_ACCEPTOR_HPP

#include "eveio/Channel.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/TcpConnection.hpp"
#include "eveio/net/TcpSocket.hpp"

#include <functional>

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

using NewTcpConnectionCallback = std::function<void(TcpConnection &&)>;

class Acceptor : public std::enable_shared_from_this<Acceptor> {
  EventLoop *loop;
  volatile bool is_listening;
  TcpSocket accept_socket;
  Channel channel;
  NewTcpConnectionCallback callback;

public:
  Acceptor(EventLoop &loop, TcpSocket &&socket, bool reuse_port) noexcept;

  Acceptor(const Acceptor &) = delete;
  Acceptor &operator=(const Acceptor &) = delete;

  Acceptor(Acceptor &&) = delete;
  Acceptor &operator=(Acceptor &&) = delete;

  ~Acceptor() noexcept;

  template <
      class Fn,
      class = std::enable_if_t<std::is_constructible<NewTcpConnectionCallback,
                                                     std::decay_t<Fn>>::value,
                               int>>
  void SetNewConnectionCallback(Fn &&fn) {
    callback = NewTcpConnectionCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetNewConnectionCallback(Fn &&fn, Args &&...args) {
    callback = std::bind(std::forward<Fn>(fn),
                         std::forward<Args>(args)...,
                         std::placeholders::_1);
  }

  bool IsListening() const noexcept { return is_listening; }

  const InetAddr &LocalAddr() const noexcept {
    return accept_socket.LocalAddr();
  }

  void Listen() noexcept;

  void Quit() noexcept;

private:
  void HandleRead(Time) noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_ACCEPTOR_HPP
