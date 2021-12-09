#include "eveio/net/AsyncTcpConnection.hpp"
#include "eveio/Channel.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/net/TcpConnection.hpp"

#include <spdlog/spdlog.h>

#include <atomic>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <utility>

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

eveio::net::AsyncTcpConnection::AsyncTcpConnection(
    EventLoop &loop, TcpConnection &&connect) noexcept
    : std::enable_shared_from_this<AsyncTcpConnection>(),
      guard_self(),
      loop(&loop),
      conn(std::move(connect)),
      channel(loop, conn.native_socket()),
      read_buffer(),
      write_buffer(),
      message_callback(),
      write_complete_callback(),
      close_callback(),
      is_exiting(false) {}

eveio::net::AsyncTcpConnection::~AsyncTcpConnection() noexcept {
  // assert(loop->IsInLoopThread());
  assert(channel.IsNoneEvent());
}

void eveio::net::AsyncTcpConnection::Initialize() noexcept {
  if (!conn.SetNonblock(true)) {
    SPDLOG_CRITICAL(
        "failed to set connection {} nonblock with peer address: {}. Abort.",
        conn.native_socket(),
        PeerAddr().GetIpWithPort());
    std::abort();
  }

  if (!conn.SetKeepAlive(true)) {
    SPDLOG_CRITICAL(
        "failed to set connection {} keepalive with peer address: {}. Abort.",
        conn.native_socket(),
        PeerAddr().GetIpWithPort());
    std::abort();
  }

  guard_self = shared_from_this();
  channel.Tie(guard_self);
  channel.SetReadCallback(&AsyncTcpConnection::HandleRead, this);
  channel.SetWriteCallback(&AsyncTcpConnection::SendInLoop, this);
  channel.SetCloseCallback([this]() { this->WouldDestroy(); });
  loop->RunInLoop(&Channel::EnableReading, &channel);
}

void eveio::net::AsyncTcpConnection::AsyncSend(StringRef data) noexcept {
  AsyncSend(data.data(), data.size());
}

void eveio::net::AsyncTcpConnection::AsyncSend(const void *buf,
                                               size_t byte) noexcept {
  if (is_exiting.load(std::memory_order_relaxed))
    return;

  if (loop->IsInLoopThread()) {
    write_buffer.Append(buf, byte);
    SendInLoop();
  } else {
    String data(static_cast<const char *>(buf), byte);
    loop->RunInLoop([this, data]() {
      this->write_buffer.Append(data.data(), data.size());
      this->channel.EnableWriting();
    });
  }
}

void eveio::net::AsyncTcpConnection::WouldDestroy() noexcept {
  if (is_exiting.exchange(true, std::memory_order_relaxed) == false) {
    loop->QueueInLoop(&AsyncTcpConnection::Destroy, shared_from_this());
  }
}

void eveio::net::AsyncTcpConnection::Destroy() noexcept {
  assert(loop->IsRunning());
  assert(loop->IsInLoopThread());

  auto guard = shared_from_this();

  if (close_callback)
    close_callback(this);

  channel.DisableAll();
  channel.Unregist();
  this->guard_self.reset();
}

void eveio::net::AsyncTcpConnection::HandleRead(Time time) noexcept {
  assert(loop->IsInLoopThread());
  int byte_read = 0;

  auto try_read = [this, time]() {
    if (read_buffer.Size() > 0) {
      if (message_callback)
        message_callback(this, read_buffer, time);
      else
        read_buffer.Clear();
    }
  };

  if (read_buffer.ReadFromSocket(conn.native_socket(), byte_read)) {
    assert(byte_read >= 0);
    assert(guard_self != nullptr);

    try_read();
  } else {
    int saved_errno = errno;
    if (saved_errno == ECONNRESET || saved_errno == EPIPE) {
      try_read();
      SPDLOG_TRACE("Connection with {} closed.", PeerAddr().GetIpWithPort());
      WouldDestroy();
    } else if (saved_errno != EWOULDBLOCK) {
      SPDLOG_ERROR("connection {} failed to receive data from {}: {}.",
                   conn.native_socket(),
                   PeerAddr().GetIpWithPort(),
                   std::strerror(saved_errno));
    }
  }
}

void eveio::net::AsyncTcpConnection::SendInLoop() noexcept {
  assert(loop->IsInLoopThread());

  int byte_write = conn.Send(write_buffer.Data<char>(), write_buffer.Size());
  if (byte_write >= 0) {
    write_buffer.Readout(byte_write);
    if (write_buffer.IsEmpty()) {
      channel.DisableWriting();
      if (write_complete_callback) {
        // get shared_ptr to avoid this from being released
        auto p = shared_from_this();
        loop->QueueInLoop(
            [this, p]() { this->write_complete_callback(p.get()); });
      }
    }
  } else {
    int saved_errno = errno;
    if (saved_errno != EWOULDBLOCK) {
      if (saved_errno == ECONNRESET || saved_errno == EPIPE) {
        SPDLOG_TRACE("Connection with {} closed.", PeerAddr().GetIpWithPort());
        WouldDestroy();
        return;
      } else {
        SPDLOG_ERROR("Connection {} failed to transfer data to peer {}. error "
                     "message: {}.",
                     conn.native_socket(),
                     PeerAddr().GetIpWithPort(),
                     std::strerror(errno));
      }
    }
  }

  if (!write_buffer.IsEmpty())
    channel.EnableWriting();
}
