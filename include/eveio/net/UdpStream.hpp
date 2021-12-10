#ifndef EVEIO_NET_UDPSTREAM_HPP
#define EVEIO_NET_UDPSTREAM_HPP

#include "eveio/Result.hpp"
#include "eveio/net/Socket.hpp"

namespace eveio {
namespace net {

class UdpStream {
public:
  typedef detail::native_socket_type native_socket_type;
  static constexpr const native_socket_type InvalidSocket =
      detail::InvalidSocket;

private:
  native_socket_type sock;

  UdpStream(native_socket_type s) noexcept : sock(s) {}

public:
  static Result<UdpStream> Ipv4Stream() noexcept;
  static Result<UdpStream> Ipv6Stream() noexcept;

  UdpStream(const UdpStream &) = delete;
  UdpStream &operator=(const UdpStream &) = delete;

  UdpStream(UdpStream &&) noexcept;
  UdpStream &operator=(UdpStream &&) noexcept;

  ~UdpStream() noexcept;

  int64_t SendTo(StringRef data, const InetAddr &target) const noexcept;
  int64_t
  SendTo(const void *data, size_t size, const InetAddr &target) const noexcept;
  int64_t ReceiveFrom(void *buf, size_t cap, InetAddr &peer) const noexcept;

  bool SetNonblock(bool on) const noexcept;

  native_socket_type native_socket() const noexcept { return sock; }
  operator native_socket_type() const noexcept { return sock; }
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_UDPSTREAM_HPP
