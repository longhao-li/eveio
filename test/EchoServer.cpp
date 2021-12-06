#include "eveio/Channel.hpp"
#include "eveio/EventLoopThread.hpp"
#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/TcpServer.hpp"

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <memory>

using namespace eveio;
using namespace eveio::net;

// used to block main thread.
static EventLoop *GlobalLoop;
static int NumThread;

class EchoServer {
  EventLoopThreadPool *io_context;
  TcpServer server;

public:
  EchoServer(EventLoopThreadPool &io_context,
             const InetAddr listen_addr) noexcept
      : io_context(&io_context), server(io_context, listen_addr, false) {
    server.SetConnectionCallback(&EchoServer::OnConnection, this);
    server.SetMessageCallback(&EchoServer::OnMessage, this);
    server.SetCloseCallback(&EchoServer::OnClose, this);
  }

  void Start() noexcept { server.Start(); }

private:
  void OnConnection(std::shared_ptr<AsyncTcpConnection> conn) noexcept {
    SPDLOG_INFO("{} -> {} is {}.",
                conn->PeerAddr().GetIpWithPort(),
                server.LocalAddr().GetIpWithPort(),
                conn->IsClosed() ? "down" : "up");
    conn->AsyncSend("Hello\n");
  }

  void OnMessage(std::shared_ptr<AsyncTcpConnection> conn,
                 Buffer &buf,
                 Time time) noexcept {
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
      GlobalLoop->Quit();
    }
    conn->AsyncSend(msg);
  }

  void OnClose(std::shared_ptr<AsyncTcpConnection> conn) noexcept {
    SPDLOG_INFO("Connection with {} is closing.",
                conn->PeerAddr().GetIpWithPort());
  }
};

int main(int argc, char *argv[]) {
  SPDLOG_INFO("sizeof AsyncTcpConnection: {}.", sizeof(AsyncTcpConnection));
  if (argc > 1)
    NumThread = atoi(argv[1]);
  EventLoop loop;
  GlobalLoop = &loop;
  InetAddr addr = InetAddr::Ipv4Any(2000);
  EventLoopThreadPool io_context;
  EchoServer server(io_context, addr);
  server.Start();
  loop.Loop(); // stupid design.
}
