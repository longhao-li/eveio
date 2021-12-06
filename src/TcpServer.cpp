#include "eveio/net/TcpServer.hpp"
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

eveio::net::TcpServer::TcpServer(EventLoopThreadPool &io_context,
                                 const InetAddr &listen_addr,
                                 bool reuse_addr) noexcept
    : io_context(&io_context),
      is_started(false),
      reuse_port(reuse_addr),
      acceptor(),
      connection_callback(),
      message_callback(),
      write_complete_callback(),
      close_callback(),
      local_addr(listen_addr) {}

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

    EventLoop *loop = io_context->GetNextLoop();
    assert(loop != nullptr);

    acceptor = std::make_unique<Acceptor>(
        *loop, std::move(sock_res.GetValue()), reuse_port);

    acceptor->SetNewConnectionCallback([this](TcpConnection &&conn) {
      auto async_conn =
          std::shared_ptr<AsyncTcpConnection>(new AsyncTcpConnection(
              *this->io_context->GetNextLoop(), std::move(conn)));
      async_conn->Initialize();

      if (this->message_callback)
        async_conn->SetMessageCallback(this->message_callback);
      if (this->write_complete_callback)
        async_conn->SetWriteCompleteCallback(this->write_complete_callback);
      if (this->close_callback)
        async_conn->SetCloseCallback(this->close_callback);
      if (connection_callback)
        connection_callback(async_conn);
    });

    acceptor->Listen();
  }
}
