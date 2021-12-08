#ifndef EVEIO_NET_TCPSERVER_HPP
#define EVEIO_NET_TCPSERVER_HPP

#include "eveio/Channel.hpp"
#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/SmartPtr.hpp"
#include "eveio/net/Acceptor.hpp"
#include "eveio/net/AsyncTcpConnection.hpp"
#include "eveio/net/InetAddr.hpp"

#include <atomic>
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

class TcpServer {
  EventLoop *const loop;
  EventLoopThreadPool *const io_context;
  std::atomic_bool is_started;
  bool reuse_port;

  SharedPtr<Acceptor> acceptor;

  TcpConnectionCallback connection_callback;
  TcpMessageCallback message_callback;
  TcpWriteCompleteCallback write_complete_callback;
  TcpCloseCallback close_callback;

  InetAddr local_addr;

public:
  TcpServer(EventLoop &loop,
            EventLoopThreadPool &io_context,
            const InetAddr &listen_addr,
            bool reuse_port) noexcept;

  TcpServer(const TcpServer &) = delete;
  TcpServer &operator=(const TcpServer &) = delete;

  TcpServer(TcpServer &&) = delete;
  TcpServer &operator=(TcpServer &&) = delete;

  ~TcpServer() noexcept;

  const InetAddr &LocalAddr() const noexcept { return acceptor->LocalAddr(); }
  EventLoop *GetLoop() const noexcept { return loop; }

  template <
      class Fn,
      class = std::enable_if_t<
          std::is_constructible<TcpConnectionCallback, std::decay_t<Fn>>::value,
          int>>
  void SetConnectionCallback(Fn &&fn) {
    connection_callback = TcpConnectionCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetConnectionCallback(Fn &&fn, Args &&...args) {
    connection_callback = std::bind(std::forward<Fn>(fn),
                                    std::forward<Args>(args)...,
                                    std::placeholders::_1);
  }

  template <
      class Fn,
      class = std::enable_if_t<
          std::is_constructible<TcpMessageCallback, std::decay_t<Fn>>::value,
          int>>
  void SetMessageCallback(Fn &&fn) {
    message_callback = TcpMessageCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetMessageCallback(Fn &&fn, Args &&...args) {
    message_callback = std::bind(std::forward<Fn>(fn),
                                 std::forward<Args>(args)...,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3);
  }

  template <
      class Fn,
      class = std::enable_if_t<std::is_constructible<TcpWriteCompleteCallback,
                                                     std::decay_t<Fn>>::value,
                               int>>
  void SetWriteCompleteCallback(Fn &&fn) {
    write_complete_callback = TcpWriteCompleteCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetWriteCompleteCallback(Fn &&fn, Args &&...args) {
    write_complete_callback = std::bind(std::forward<Fn>(fn),
                                        std::forward<Args>(args)...,
                                        std::placeholders::_1);
  }

  template <
      class Fn,
      class = std::enable_if_t<
          std::is_constructible<TcpCloseCallback, std::decay_t<Fn>>::value,
          int>>
  void SetCloseCallback(Fn &&fn) {
    close_callback = TcpCloseCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetCloseCallback(Fn &&fn, Args &&...args) {
    close_callback = std::bind(std::forward<Fn>(fn),
                               std::forward<Args>(args)...,
                               std::placeholders::_1);
  }

  void Start() noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_TCPSERVER_HPP
