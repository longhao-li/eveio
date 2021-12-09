#include "eveio/net/InetAddr.hpp"
#include "eveio/net/UdpServer.hpp"

#include <spdlog/spdlog.h>

using namespace eveio;
using namespace eveio::net;

class EchoServer {
  UdpServer server;

public:
  EchoServer(EventLoop &loop, const InetAddr &listen_addr) noexcept
      : server(loop, listen_addr, true) {
    server.SetMessageCallback([](UdpServer *srv,
                                 const InetAddr &peer,
                                 FixedBuffer &buf,
                                 Time time,
                                 int64_t ret) {
      if (ret < 0)
        SPDLOG_ERROR("failed to receive message: {}.", std::strerror(errno));
      else {
        SPDLOG_INFO("receive {} bytes data from {}: {}",
                    ret,
                    peer.GetIpWithPort(),
                    StringRef(buf.Data(), buf.Size()));

        srv->AsyncSend(buf.Data(), static_cast<size_t>(ret), peer);
      }
    });
  }

  void Start() noexcept { server.Start(); }
};

int main() {
  EventLoop loop;
  InetAddr addr = InetAddr::Ipv4Any(2000);
  EchoServer server(loop, addr);
  server.Start();
  loop.Loop();
  return 0;
}
