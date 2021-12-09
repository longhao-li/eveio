#include "eveio/net/Acceptor.hpp"
#include "eveio/Channel.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/net/TcpConnection.hpp"
#include "eveio/net/TcpSocket.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdlib>
#include <memory>

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

eveio::net::Acceptor::Acceptor(EventLoop &loop,
                               TcpSocket &&socket,
                               bool reuse_port) noexcept
    : std::enable_shared_from_this<Acceptor>(),
      loop(&loop),
      is_listening(false),
      accept_socket(std::move(socket)),
      channel(loop, accept_socket.native_socket()),
      callback() {
  if (!accept_socket.SetReuseAddr(true)) {
    SPDLOG_CRITICAL("socket {} failed to set reuse addr. abort.",
                    accept_socket.native_socket());
    std::abort();
  }

  if (reuse_port && !accept_socket.SetReusePort(true)) {
    SPDLOG_CRITICAL("socket {} failed to set reuse port. abort.",
                    accept_socket.native_socket());
    std::abort();
  }

  if (!accept_socket.SetNonblock(true)) {
    SPDLOG_CRITICAL("socket {} failed to set nonblock. abort.",
                    accept_socket.native_socket());
    std::abort();
  }

  channel.SetReadCallback(&Acceptor::HandleRead, this);
}

eveio::net::Acceptor::~Acceptor() noexcept { assert(channel.IsNoneEvent()); }

void eveio::net::Acceptor::Listen() noexcept {
  assert(loop->IsInLoopThread());
  is_listening = true;
  if (!accept_socket.Listen(SOMAXCONN)) {
    SPDLOG_CRITICAL("socket {} failed to listen. abort.",
                    accept_socket.native_socket());
    std::abort();
  }
  loop->RunInLoop(&Channel::EnableReading, &channel);
}

void eveio::net::Acceptor::Quit() noexcept {
  assert(loop->IsInLoopThread());
  channel.DisableAll();
  channel.Unregist();
  is_listening = false;
}

void eveio::net::Acceptor::HandleRead(Time) noexcept {
  assert(loop->IsInLoopThread());
  auto conn_res = accept_socket.Accept();
  if (!conn_res.IsValid()) {
    SPDLOG_ERROR("acceptor {} failed to accept: {}.",
                 accept_socket.native_socket(),
                 conn_res.GetError());
  } else if (callback) {
    callback(std::move(conn_res.Unwarp()));
  }
}
