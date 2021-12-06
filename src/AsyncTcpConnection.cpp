#include "eveio/net/AsyncTcpConnection.hpp"
#include "eveio/Channel.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/net/TcpConnection.hpp"

#include <spdlog/spdlog.h>

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <utility>

using namespace eveio;
using namespace eveio::net;

eveio::net::AsyncTcpConnection::AsyncTcpConnection(
    EventLoop &loop, TcpConnection &&connect) noexcept
    : std::enable_shared_from_this<AsyncTcpConnection>(),
      guard_self(),
      loop(&loop),
      conn(std::move(connect)),
      channel(loop, conn.native_socket()),
      read_buffer(),
      write_buffer(),
      message_callback(),
      write_complete_callback(),
      close_callback() {}

eveio::net::AsyncTcpConnection::~AsyncTcpConnection() noexcept {
  assert(loop->IsInLoopThread());
  assert(channel.IsNoneEvent());
  SPDLOG_TRACE(
      "AsyncTcpConnection with socket:{} peer addr: {} is destructing.",
      conn.native_socket(),
      PeerAddr().GetIpWithPort());
}

void eveio::net::AsyncTcpConnection::Initialize() noexcept {
  if (!conn.SetNonblock(true)) {
    // SPDLOG_CRITICAL(
    //     "failed to set connection {} nonblock with peer address: {}. Abort.",
    //     conn.native_socket(),
    //     PeerAddr().GetIpWithPort());
    // std::abort();
  }

  guard_self = shared_from_this();
  channel.Tie(guard_self);
  channel.SetReadCallback(&AsyncTcpConnection::HandleRead, this);
  channel.SetWriteCallback(&AsyncTcpConnection::SendInLoop, this);
  channel.SetCloseCallback([this]() { this->Destroy(); });
  channel.EnableReading();
}

void eveio::net::AsyncTcpConnection::AsyncSend(StringRef data) noexcept {
  AsyncSend(data.data(), data.size());
}

void eveio::net::AsyncTcpConnection::AsyncSend(const void *buf,
                                               size_t byte) noexcept {
  if (loop->IsInLoopThread()) {
    this->write_buffer.Append(buf, byte);
    SendInLoop();
  } else {
    String data(static_cast<const char *>(buf), byte);
    loop->RunInLoop([this, data]() {
      this->write_buffer.Append(data.data(), data.size());
      this->channel.EnableWriting();
    });
  }
}

void eveio::net::AsyncTcpConnection::WouldDestroy() noexcept {
  loop->QueueInLoop(&AsyncTcpConnection::Destroy, shared_from_this());
}

void eveio::net::AsyncTcpConnection::Destroy() noexcept {
  assert(loop->IsRunning());
  if (!loop->IsInLoopThread()) {
    loop->RunInLoop(
        std::bind(&AsyncTcpConnection::Destroy, shared_from_this()));
  } else if (is_exiting.exchange(true, std::memory_order_relaxed) == false) {
    auto guard = shared_from_this();
    channel.Unregist();

    SPDLOG_TRACE("AsyncTcpConnection {} with peer: {} is closing.",
                 conn.native_socket(),
                 PeerAddr().GetIpWithPort());

    if (close_callback)
      close_callback(shared_from_this());
    this->guard_self.reset();
  }
}

void eveio::net::AsyncTcpConnection::HandleRead(Time time) noexcept {
  assert(loop->IsInLoopThread());
  int byte_read = read_buffer.ReadFromSocket(conn.native_socket());

  if (byte_read > 0) {
    assert(read_buffer.Size() > 0);
    if (message_callback)
      message_callback(shared_from_this(), read_buffer, time);
    else
      read_buffer.Clear();
  } else {
    if (byte_read == 0 || (byte_read < 0 && errno == ECONNRESET)) {
      if (read_buffer.Size() > 0) {
        if (message_callback)
          message_callback(shared_from_this(), read_buffer, time);
        else
          read_buffer.Clear();
      }
      SPDLOG_TRACE("Connection with {} closed.", PeerAddr().GetIpWithPort());
    } else {
      SPDLOG_ERROR("connection {} failed to receive data from {}: {}.",
                   conn.native_socket(),
                   PeerAddr().GetIpWithPort(),
                   std::strerror(errno));
    }
    WouldDestroy();
  }
}

void eveio::net::AsyncTcpConnection::SendInLoop() noexcept {
  assert(loop->IsInLoopThread());
  int byte_write = conn.Send(write_buffer.Data<char>(), write_buffer.Size());
  if (byte_write > 0) {
    write_buffer.Readout(byte_write);
    if (write_buffer.IsEmpty()) {
      channel.DisableWriting();
      if (write_complete_callback)
        loop->QueueInLoop(write_complete_callback, shared_from_this());
    }
  } else {
    SPDLOG_ERROR("Connection {} failed to transfer data to peer {}.",
                 conn.native_socket(),
                 PeerAddr().GetIpWithPort());
    WouldDestroy();
  }
}
