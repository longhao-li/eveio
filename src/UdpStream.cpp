#include "eveio/net/UdpStream.hpp"
#include "eveio/Result.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/UdpSocket.hpp"

#include <cstring>

using namespace eveio;
using namespace eveio::net;

Result<UdpStream> eveio::net::UdpStream::Ipv4Stream() noexcept {
  UdpStream::native_socket_type sock = detail::socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == UdpStream::InvalidSocket)
    return Result<UdpStream>::Error(std::strerror(errno));
  return Result<UdpStream>::Ok(UdpStream(sock));
}

Result<UdpStream> eveio::net::UdpStream::Ipv6Stream() noexcept {
  UdpStream::native_socket_type sock = detail::socket(AF_INET6, SOCK_DGRAM, 0);
  if (sock == UdpStream::InvalidSocket)
    return Result<UdpStream>::Error(std::strerror(errno));
  return Result<UdpStream>::Ok(UdpStream(sock));
}

eveio::net::UdpStream::UdpStream(UdpStream &&other) noexcept
    : sock(other.sock) {
  other.sock = InvalidSocket;
}

UdpStream &eveio::net::UdpStream::operator=(UdpStream &&other) noexcept {
  if (sock != InvalidSocket)
    detail::close_socket(sock);
  sock = other.sock;
  other.sock = InvalidSocket;
  return (*this);
}

eveio::net::UdpStream::~UdpStream() noexcept {
  if (sock != InvalidSocket)
    detail::close_socket(sock);
}

int64_t eveio::net::UdpStream::SendTo(StringRef data,
                                      const InetAddr &target) const noexcept {
  return SendTo(data.data(), data.size(), target);
}

int64_t eveio::net::UdpStream::SendTo(const void *data,
                                      size_t size,
                                      const InetAddr &target) const noexcept {
  return detail::socket_sendto(sock,
                               data,
                               size,
                               target.AsSockaddr(),
                               static_cast<socklen_t>(target.Size()));
}

int64_t eveio::net::UdpStream::ReceiveFrom(void *buf,
                                           size_t cap,
                                           InetAddr &peer) const noexcept {
  struct sockaddr *peer_addr = const_cast<struct sockaddr *>(peer.AsSockaddr());
  socklen_t len = sizeof(peer);
  return detail::socket_recvfrom(sock, buf, cap, peer_addr, &len);
}

bool eveio::net::UdpStream::SetNonblock(bool on) const noexcept {
  return detail::set_nonblock(sock, on);
}
