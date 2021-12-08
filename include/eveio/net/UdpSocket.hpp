#ifndef EVEIO_NET_UDPSOCKET_HPP
#define EVEIO_NET_UDPSOCKET_HPP

#include "eveio/String.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/Socket.hpp"

namespace eveio {
namespace net {

class UdpSocket : public detail::SocketBase {
  UdpSocket(native_socket_type h, const InetAddr &addr) noexcept
      : detail::SocketBase(h, addr) {}

public:
  static Result<UdpSocket> Create(const InetAddr &local_addr) noexcept;

  UdpSocket(UdpSocket &&) noexcept = default;
  UdpSocket &operator=(UdpSocket &&) noexcept = default;

  ~UdpSocket() noexcept = default;

  int SendTo(StringRef data, const InetAddr &target) const noexcept;
  int SendTo(const void *data,
             size_t size,
             const InetAddr &target) const noexcept;
  int ReceiveFrom(void *buf, size_t cap, InetAddr &peer) const noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_UDPSOCKET_HPP
