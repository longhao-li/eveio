/// A very simple http server example.
#include "eveio/TcpServer.h"

#include <iostream>

using namespace eveio;

static const std::string HttpResponse =
    R"(<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>Hello!</title>
  </head>
  <body>
    <h1>Hello!</h1>
    <p>Hello, world!</p>
  </body>
</html>
)";

int main() {
    auto local_addr = InetAddr::Ipv4Any(8080);
    assert(local_addr.IsValid());
    EventLoop loop;

    TcpServer tcp_server(loop, local_addr);
    tcp_server.SetConnectionCallback([](AsyncTcpConnection *conn) {
        InetAddr addr;
        if (conn->GetPeerAddr(addr)) {
            std::cout << "Connection with " << addr.GetIpWithPort()
                      << " established.\n";
        }
    });

    tcp_server.SetMessageCallback(
        [](AsyncTcpConnection *conn, AsyncTcpConnBuffer &buffer) {
            buffer.Clear();
            std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " +
                                   std::to_string(HttpResponse.size()) +
                                   "\r\n\r\n" + HttpResponse;
            conn->AsyncSend(response.c_str(), response.size());
        });

    tcp_server.Start();
    loop.Loop();

    return 0;
}
