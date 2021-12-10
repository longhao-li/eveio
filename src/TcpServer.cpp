#include "eveio/net/TcpServer.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/SmartPtr.hpp"
#include "eveio/net/Acceptor.hpp"
#include "eveio/net/AsyncTcpConnection.hpp"
#include "eveio/net/TcpSocket.hpp"

#include <spdlog/spdlog.h>

#include <atomic>
#include <cstddef>
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

eveio::net::TcpServer::TcpServer(EventLoop &event_loop,
                                 EventLoopThreadPool &thread_poll,
                                 const InetAddr &listen_addr,
                                 bool reuse_addr) noexcept
    : loop(&event_loop),
      io_context(&thread_poll),
      is_started(false),
      reuse_port(reuse_addr),
      acceptor(),
      connection_callback(),
      message_callback(),
      write_complete_callback(),
      close_callback(),
      local_addr(listen_addr) {}

eveio::net::TcpServer::~TcpServer() noexcept {
  loop->RunInLoop(&Acceptor::Quit, acceptor);
}

void eveio::net::TcpServer::Start() noexcept {
  if (is_started.exchange(true, std::memory_order_relaxed) == false) {
    auto sock_res = TcpSocket::Create(local_addr);
    if (!sock_res.IsValid()) {
      SPDLOG_CRITICAL("failed to create tcp socket: {}. listen addr: {}.",
                      sock_res.GetError(),
                      local_addr.GetIpWithPort());
      std::abort();
    }

    io_context->Start();

    acceptor =
        MakeShared<Acceptor>(*loop, std::move(sock_res.Unwarp()), reuse_port);

    acceptor->SetNewConnectionCallback([this](TcpConnection &&conn) {
      auto async_conn = MakeShared<AsyncTcpConnection>(
          *(this->io_context->GetNextLoop()), std::move(conn));
      async_conn->Initialize();

      if (this->message_callback)
        async_conn->SetMessageCallback(this->message_callback);
      if (this->write_complete_callback)
        async_conn->SetWriteCompleteCallback(this->write_complete_callback);
      if (this->close_callback)
        async_conn->SetCloseCallback(this->close_callback);
      if (connection_callback)
        connection_callback(async_conn.get());
    });

    loop->RunInLoop([this]() { acceptor->Listen(); });
  }
}
