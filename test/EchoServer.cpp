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
    if (msg == "exit\n") {
      conn->AsyncSend("bye\n");
      conn->CloseWrite();
    }

    if (msg == "quit\n") {
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
