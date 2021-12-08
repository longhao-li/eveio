#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/net/AsyncTcpConnection.hpp"
#include "eveio/net/Buffer.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/TcpServer.hpp"

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <cstdio>
#include <cstdlib>

using namespace eveio;

class EchoTcpServer {
  net::TcpServer server;

public:
  EchoTcpServer(EventLoop &loop,
                EventLoopThreadPool &io_context,
                const net::InetAddr &addr)
      : server(loop, io_context, addr, false) {
    server.SetConnectionCallback(&EchoTcpServer::OnConnection, this);
    server.SetMessageCallback(&EchoTcpServer::OnMessage, this);
    server.Start();
  }

  void OnConnection(net::AsyncTcpConnection *conn) noexcept {
    conn->SetKeepAlive(true);
  }
  void OnMessage(net::AsyncTcpConnection *conn,
                 net::Buffer &input,
                 Time time) noexcept {
    auto str = input.RetrieveAsString();
    conn->AsyncSend(str);
  }
};

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: cmd port\n");
    return -10;
  }

  int port = atoi(argv[1]);
  spdlog::default_logger()->set_level(spdlog::level::err);

  EventLoop loop;
  EventLoopThreadPool io_context(std::thread::hardware_concurrency());
  net::InetAddr addr = net::InetAddr::Ipv4Any(port);
  EchoTcpServer server(loop, io_context, addr);
  loop.Loop();
  return 0;
}
