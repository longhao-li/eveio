#include "eveio/net/UdpSocket.hpp"
#include "eveio/Result.hpp"
#include "eveio/net/Socket.hpp"

using namespace eveio;
using namespace eveio::net;

Result<UdpSocket> eveio::net::UdpSocket::Create(const InetAddr &addr) noexcept {
  native_socket_type sock = detail::socket(addr.GetFamily(), SOCK_DGRAM, 0);
  if (sock == InvalidSocket)
    return Result<UdpSocket>::Error(std::strerror(errno));

  if (!detail::bind(sock, addr.AsSockaddr(), addr.Size())) {
    int saved_errno = errno;
    detail::close_socket(sock);
    return Result<UdpSocket>::Error(std::strerror(saved_errno));
  }
  return Result<UdpSocket>::Ok(UdpSocket(sock, addr));
}

int eveio::net::UdpSocket::SendTo(StringRef data,
                                  const InetAddr &target) const noexcept {
  return SendTo(data.data(), data.size(), target);
}

int eveio::net::UdpSocket::SendTo(const void *data,
                                  size_t size,
                                  const InetAddr &target) const noexcept {
  return detail::socket_sendto(
      sock, data, size, target.AsSockaddr(), target.Size());
}

int eveio::net::UdpSocket::ReceiveFrom(void *buf,
                                       size_t cap,
                                       InetAddr &peer) const noexcept {
  struct sockaddr *peer_addr = const_cast<struct sockaddr *>(peer.AsSockaddr());
  socklen_t len = sizeof(peer);
  return detail::socket_recvfrom(sock, buf, cap, peer_addr, &len);
}
