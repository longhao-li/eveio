#ifndef EVEIO_NET_TCPCONNECTION_HPP
#define EVEIO_NET_TCPCONNECTION_HPP

#include "eveio/Result.hpp"
#include "eveio/String.hpp"
#include "eveio/Time.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/Socket.hpp"

namespace eveio {
namespace net {

class TcpConnection {
  native_socket_type conn_handle;
  InetAddr peer_addr;
  Time create_time;

  TcpConnection(native_socket_type conn, const InetAddr &addr) noexcept
      : conn_handle(conn), peer_addr(addr), create_time(Time::Now()) {}

public:
  static Result<TcpConnection> Connect(const InetAddr &peer) noexcept;

  TcpConnection(const TcpConnection &) = delete;
  TcpConnection &operator=(const TcpConnection &) = delete;

  TcpConnection(TcpConnection &&other) noexcept;
  TcpConnection &operator=(TcpConnection &&other) noexcept;

  ~TcpConnection() noexcept;

  Time CreateTime() const noexcept { return create_time; }
  const InetAddr &PeerAddr() const noexcept { return peer_addr; }

  int64_t Send(StringRef data) const noexcept;
  int64_t Send(const void *buf, size_t byte) const noexcept;
  int64_t Receive(void *buf, size_t cap) const noexcept;

  void CloseWrite() const noexcept;
  bool IsClosed() const noexcept;

  bool SetNoDelay(bool on) const noexcept;
  bool SetNonblock(bool on) const noexcept;
  bool SetKeepAlive(bool on) const noexcept;

  native_socket_type native_socket() const noexcept { return conn_handle; }
  operator native_socket_type() const noexcept { return conn_handle; }

  friend class TcpSocket;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_TCPCONNECTION_HPP
