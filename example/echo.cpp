#include "eveio/TcpServer.h"

using eveio::AsyncTcpConnBuffer;
using eveio::AsyncTcpConnection;
using eveio::EventLoop;
using eveio::InetAddr;
using eveio::TcpServer;

class EchoTcpServer {
public:
    EchoTcpServer(EventLoop &loop, const InetAddr &addr)
        : m_server(loop, addr) {
        m_server.SetConnectionCallback([](AsyncTcpConnection *) {});
        m_server.SetMessageCallback(
            [](AsyncTcpConnection *conn, AsyncTcpConnBuffer &buffer) {
                auto data = buffer.RetrieveAsString();
                conn->AsyncSend(data.data(), data.size());
            });
    }

    void Start() { m_server.Start(); }

private:
    TcpServer m_server;
};

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s port\n", argv[0]);
        return -10;
    }

    auto port = static_cast<uint16_t>(atoi(argv[1]));

    EventLoop     loop;
    auto          addr = InetAddr::Ipv4Any(port);
    EchoTcpServer server(loop, addr);
    server.Start();
    loop.Loop();
    return 0;
}
