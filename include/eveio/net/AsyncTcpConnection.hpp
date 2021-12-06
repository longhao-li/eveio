#ifndef EVEIO_NET_ASYNC_TCPCONNECTION_HPP
#define EVEIO_NET_ASYNC_TCPCONNECTION_HPP

#include "eveio/Channel.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/String.hpp"
#include "eveio/net/Buffer.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/TcpConnection.hpp"

#include <atomic>
#include <functional>
#include <memory>

namespace eveio {
namespace net {

class AsyncTcpConnection;

using TcpMessageCallback =
    std::function<void(std::shared_ptr<AsyncTcpConnection>, Buffer &, Time)>;
using TcpWriteCompleteCallback =
    std::function<void(std::shared_ptr<AsyncTcpConnection>)>;
using TcpCloseCallback =
    std::function<void(std::shared_ptr<AsyncTcpConnection>)>;
using TcpConnectionCallback =
    std::function<void(std::shared_ptr<AsyncTcpConnection>)>;

class AsyncTcpConnection
    : public std::enable_shared_from_this<AsyncTcpConnection> {
  std::shared_ptr<AsyncTcpConnection> guard_self;
  EventLoop *const loop;

  TcpConnection conn;
  Channel channel;

  Buffer read_buffer;
  Buffer write_buffer;

  TcpMessageCallback message_callback;
  TcpWriteCompleteCallback write_complete_callback;
  TcpCloseCallback close_callback;

  std::atomic_bool is_exiting;

private:
  /// allow TcpServer to call the following private methods
  friend class TcpServer;

  void Initialize() noexcept;

  void SetMessageCallback(const TcpMessageCallback &cb) noexcept {
    message_callback = cb;
  }

  void SetWriteCompleteCallback(const TcpWriteCompleteCallback &cb) noexcept {
    write_complete_callback = cb;
  }

  void SetCloseCallback(const TcpCloseCallback &cb) noexcept {
    close_callback = cb;
  }

public:
  AsyncTcpConnection(EventLoop &loop, TcpConnection &&connect) noexcept;

  AsyncTcpConnection(const AsyncTcpConnection &) = delete;
  AsyncTcpConnection &operator=(const AsyncTcpConnection &) = delete;

  AsyncTcpConnection(AsyncTcpConnection &&) = delete;
  AsyncTcpConnection &operator=(AsyncTcpConnection &&) = delete;

  ~AsyncTcpConnection() noexcept;

  EventLoop *GetLoop() const noexcept { return loop; };

  const InetAddr &PeerAddr() const noexcept { return conn.PeerAddr(); }

  void CloseWrite() const noexcept { conn.CloseWrite(); }
  bool IsClosed() const noexcept { return conn.IsClosed(); }

  bool SetNoDelay(bool on) const noexcept { return conn.SetNoDelay(on); }
  bool SetKeepAlive(bool on) const noexcept { return conn.SetKeepAlive(on); }

  void AsyncSend(StringRef data) noexcept;
  void AsyncSend(const void *buf, size_t byte) noexcept;

  void WouldDestroy() noexcept;

private:
  void Destroy() noexcept;
  void HandleRead(Time time) noexcept;
  void SendInLoop() noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_ASYNC_TCPCONNECTION_HPP
