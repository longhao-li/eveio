#include "eveio/Channel.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/EventLoopThread.hpp"
#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/Result.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/TcpServer.hpp"

#include <fmt/ostream.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <memory>
#include <thread>

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

static int NumThread;

class EchoServer {
  EventLoopThreadPool *io_context;
  TcpServer server;

public:
  EchoServer(EventLoop &loop,
             EventLoopThreadPool &io_context,
             const InetAddr listen_addr) noexcept
      : io_context(&io_context), server(loop, io_context, listen_addr, false) {
    server.SetConnectionCallback(&EchoServer::OnConnection, this);
    server.SetMessageCallback(&EchoServer::OnMessage, this);
    server.SetCloseCallback(&EchoServer::OnClose, this);
  }

  void Start() noexcept { server.Start(); }

private:
  void OnConnection(AsyncTcpConnection *conn) noexcept {
    SPDLOG_INFO("{} -> {} is {}. socket: {}.",
                conn->PeerAddr().GetIpWithPort(),
                server.LocalAddr().GetIpWithPort(),
                conn->IsClosed() ? "down" : "up",
                conn->native_socket());
    conn->AsyncSend("Hello\n");
  }

  void OnMessage(AsyncTcpConnection *conn, Buffer &buf, Time time) noexcept {
    String msg(buf.RetrieveAsString());
    SPDLOG_INFO("{} sent {} bytes at {}.",
                conn->PeerAddr().GetIpWithPort(),
                msg.size(),
                time);
    if (msg >= "exit\n") {
      conn->AsyncSend("bye\n");
      conn->CloseWrite();
    }

    if (msg >= "quit\n") {
      server.GetLoop()->Quit();
    }
    conn->AsyncSend(msg);
  }

  void OnClose(AsyncTcpConnection *conn) noexcept {
    // SPDLOG_INFO("Connection with {} is closing.",
    //             conn->PeerAddr().GetIpWithPort());
  }
};

int main(int argc, char *argv[]) {
  SPDLOG_INFO("sizeof AsyncTcpConnection: {}. tid: {}.",
              sizeof(AsyncTcpConnection),
              std::this_thread::get_id());

  spdlog::set_default_logger(spdlog::basic_logger_mt("default", "EchoServer.log", true));

  if (argc > 1)
    NumThread = atoi(argv[1]);

  EventLoop loop;
  InetAddr addr = InetAddr::Ipv4Any(2000);
  EventLoopThreadPool io_context;
  if (NumThread > 0)
    io_context.SetThreadNum(NumThread);
  EchoServer server(loop, io_context, addr);
  server.Start();
  loop.Loop();
}
