#ifndef EVEIO_NET_TCPSOCKET_HPP
#define EVEIO_NET_TCPSOCKET_HPP

#include "eveio/Handle.hpp"
#include "eveio/Result.hpp"
#include "eveio/net/Config.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/TcpConnection.hpp"

namespace eveio {
namespace net {

class TcpSocket {
public:
  typedef detail::native_socket_type native_socket_type;
  static constexpr native_socket_type InvalidSocket = detail::InvalidSocket;

private:
  native_socket_type sock;
  InetAddr local_addr;

  TcpSocket(native_socket_type h, InetAddr addr) noexcept
      : sock(h), local_addr(addr) {}

public:
  static Result<TcpSocket> Create(const InetAddr &local_addr) noexcept;

  TcpSocket(const TcpSocket &) = delete;
  TcpSocket &operator=(const TcpSocket &) = delete;

  TcpSocket(TcpSocket &&other) noexcept;
  TcpSocket &operator=(TcpSocket &&other) noexcept;

  ~TcpSocket() noexcept;

  bool Listen(int n) const noexcept;
  void CloseWrite() const noexcept;
  void SetReuseAddr(bool on) const noexcept;
  void SetReusePort(bool on) const noexcept;

  Result<TcpConnection, const char *> Accept() const noexcept;

  const InetAddr &LocalAddr() const noexcept { return local_addr; }

  native_socket_type native_socket() const noexcept { return sock; }

  operator native_socket_type() const noexcept { return sock; }
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_TCPSOCKET_HPP
