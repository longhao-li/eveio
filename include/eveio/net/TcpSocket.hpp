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

class TcpSocket : public detail::SocketBase {
  TcpSocket(native_socket_type h, const InetAddr &addr) noexcept
      : detail::SocketBase(h, addr) {}

public:
  static Result<TcpSocket> Create(const InetAddr &local_addr) noexcept;

  TcpSocket(TcpSocket &&) noexcept = default;
  TcpSocket &operator=(TcpSocket &&) noexcept = default;

  ~TcpSocket() noexcept = default;

  bool Listen(int n) const noexcept;
  void CloseWrite() const noexcept;

  Result<TcpConnection> Accept() const noexcept;

  const InetAddr &LocalAddr() const noexcept { return local_addr; }

  native_socket_type native_socket() const noexcept { return sock; }

  operator native_socket_type() const noexcept { return sock; }
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_TCPSOCKET_HPP
