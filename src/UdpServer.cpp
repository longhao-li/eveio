#include "eveio/net/UdpServer.hpp"
#include "eveio/SmartPtr.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/UdpSocket.hpp"

#include <spdlog/spdlog.h>

#include <atomic>
#include <cstdlib>

using namespace eveio;
using namespace eveio::net;

eveio::net::UdpServer::UdpServer(EventLoop &event_loop,
                                 const InetAddr &listen_addr,
                                 bool reuse_port) noexcept
    : loop(&event_loop),
      is_started(false),
      sock(),
      channel(),
      message_callback(),
      write_complete_callback() {
  auto udp_result = UdpSocket::Create(listen_addr);
  if (!udp_result.IsValid()) {
    SPDLOG_CRITICAL("failed to create Udp socket: {}.", udp_result.GetError());
    std::abort();
  }

  sock = MakeUnique<UdpSocket>(std::move(udp_result.Unwarp()));

  if (!sock->SetReuseAddr(true)) {
    SPDLOG_CRITICAL("failed to set Udp socket {} reuse addr: {}.",
                    sock->native_socket(),
                    std::strerror(errno));
    std::abort();
  }

  if (reuse_port) {
    if (!sock->SetReusePort(true)) {
      SPDLOG_CRITICAL("failed to set Udp socket {} reuse port: {}.",
                      sock->native_socket(),
                      std::strerror(errno));
      std::abort();
    }
  }

  channel = MakeShared<Channel>(*loop, sock->native_socket());
  channel->SetReadCallback([this](Time time) {
    FixedBuffer buf;
    InetAddr addr = InetAddr::Ipv4Any(0);
    int64_t ret = this->sock->ReceiveFrom(buf.Data(), buf.Capacity(), addr);
    if (ret >= 0)
      buf.SetSize(static_cast<size_t>(ret));
    this->message_callback(this, addr, buf, time, ret);
  });

  channel->SetWriteCallback([this]() {
    Vector<std::pair<FixedBuffer, InetAddr>> temp;
    {
      std::lock_guard<std::mutex> lock(this->mutex);
      write_buffer.swap(temp);
    }

    for (auto &i : temp) {
      int64_t ret =
          this->sock->SendTo(i.first.Data(), i.first.Size(), i.second);
      if (this->write_complete_callback)
        this->write_complete_callback(this, i.second, ret);
    }

    this->loop->RunInLoop([this]() { this->channel->DisableWriting(); });
  });
}

eveio::net::UdpServer::~UdpServer() noexcept {
  loop->RunInLoop(&Channel::Unregist, channel);
}

void eveio::net::UdpServer::Start() noexcept {
  if (!is_started.exchange(true, std::memory_order_relaxed)) {
    loop->RunInLoop([this]() { this->channel->EnableReading(); });
  }
}

bool eveio::net::UdpServer::AsyncSend(StringRef data,
                                      const InetAddr &peer) noexcept {
  return AsyncSend(data.data(), data.size(), peer);
}

bool eveio::net::UdpServer::AsyncSend(const void *data,
                                      size_t sz,
                                      const InetAddr &peer) noexcept {
  if (sz > FixedBuffer::BUFFER_CAPACITY)
    return false;
  {
    std::lock_guard<std::mutex> lock(mutex);
    write_buffer.emplace_back(FixedBuffer(data, sz), peer);
  }

  loop->RunInLoop([this]() { this->channel->EnableWriting(); });
  return true;
}
