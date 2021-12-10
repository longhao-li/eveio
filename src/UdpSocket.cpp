#include "eveio/net/UdpSocket.hpp"
#include "eveio/Result.hpp"
#include "eveio/net/Socket.hpp"

#include <cstring>

using namespace eveio;
using namespace eveio::net;

Result<UdpSocket> eveio::net::UdpSocket::Create(const InetAddr &addr) noexcept {
  native_socket_type sock = socket_create(addr.GetFamily(), SOCK_DGRAM, 0);
  if (sock == INVALID_NATIVE_SOCKET)
    return Result<UdpSocket>::Error(std::strerror(errno));

  if (!socket_bind(
          sock, addr.AsSockaddr(), static_cast<socklen_t>(addr.Size()))) {
    int saved_errno = errno;
    socket_close(sock);
    return Result<UdpSocket>::Error(std::strerror(saved_errno));
  }
  return Result<UdpSocket>::Ok(UdpSocket(sock, addr));
}

int64_t eveio::net::UdpSocket::SendTo(StringRef data,
                                      const InetAddr &target) const noexcept {
  return SendTo(data.data(), data.size(), target);
}

int64_t eveio::net::UdpSocket::SendTo(const void *data,
                                      size_t size,
                                      const InetAddr &target) const noexcept {
  return socket_sendto(sock,
                       data,
                       size,
                       target.AsSockaddr(),
                       static_cast<socklen_t>(target.Size()));
}

int64_t eveio::net::UdpSocket::ReceiveFrom(void *buf,
                                           size_t cap,
                                           InetAddr &peer) const noexcept {
  struct sockaddr *peer_addr = const_cast<struct sockaddr *>(peer.AsSockaddr());
  socklen_t len = sizeof(peer);
  return socket_recvfrom(sock, buf, cap, peer_addr, &len);
}
