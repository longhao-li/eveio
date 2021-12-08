#ifndef EVEIO_NET_ASYNC_TCPCONNECTION_HPP
#define EVEIO_NET_ASYNC_TCPCONNECTION_HPP

#include "eveio/Channel.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/String.hpp"
#include "eveio/net/Buffer.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/TcpConnection.hpp"

#include <atomic>
#include <functional>
#include <memory>

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

class AsyncTcpConnection;

using TcpMessageCallback =
    std::function<void(AsyncTcpConnection *, Buffer &, Time)>;
using TcpWriteCompleteCallback = std::function<void(AsyncTcpConnection *)>;
using TcpCloseCallback = std::function<void(AsyncTcpConnection *)>;
using TcpConnectionCallback = std::function<void(AsyncTcpConnection *)>;

class AsyncTcpConnection
    : public std::enable_shared_from_this<AsyncTcpConnection> {
  SharedPtr<AsyncTcpConnection> guard_self;
  EventLoop *const loop;

  TcpConnection conn;
  Channel channel;

  Buffer read_buffer;
  Buffer write_buffer;

  TcpMessageCallback message_callback;
  TcpWriteCompleteCallback write_complete_callback;
  TcpCloseCallback close_callback;

  std::atomic_bool is_exiting;

public:
  // for internal usage
  void Initialize() noexcept;

  void SetMessageCallback(const TcpMessageCallback &cb) noexcept {
    message_callback = cb;
  }

  void SetWriteCompleteCallback(const TcpWriteCompleteCallback &cb) noexcept {
    write_complete_callback = cb;
  }

  void SetCloseCallback(const TcpCloseCallback &cb) noexcept {
    close_callback = cb;
  }

public:
  typedef detail::native_socket_type native_socket_type;

  AsyncTcpConnection(EventLoop &loop, TcpConnection &&connect) noexcept;

  AsyncTcpConnection(const AsyncTcpConnection &) = delete;
  AsyncTcpConnection &operator=(const AsyncTcpConnection &) = delete;

  AsyncTcpConnection(AsyncTcpConnection &&) = delete;
  AsyncTcpConnection &operator=(AsyncTcpConnection &&) = delete;

  ~AsyncTcpConnection() noexcept;

  EventLoop *GetLoop() const noexcept { return loop; };

  const InetAddr &PeerAddr() const noexcept { return conn.PeerAddr(); }

  void CloseWrite() const noexcept { conn.CloseWrite(); }
  bool IsClosed() const noexcept { return conn.IsClosed(); }

  bool SetNoDelay(bool on) const noexcept { return conn.SetNoDelay(on); }
  bool SetKeepAlive(bool on) const noexcept { return conn.SetKeepAlive(on); }

  void AsyncSend(StringRef data) noexcept;
  void AsyncSend(const void *buf, size_t byte) noexcept;

  void WouldDestroy() noexcept;

  native_socket_type native_socket() const noexcept {
    return conn.native_socket();
  }

private:
  void Destroy() noexcept;
  void HandleRead(Time time) noexcept;
  void SendInLoop() noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_ASYNC_TCPCONNECTION_HPP
