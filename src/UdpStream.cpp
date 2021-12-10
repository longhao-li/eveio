#include "eveio/net/UdpStream.hpp"
#include "eveio/Result.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/UdpSocket.hpp"

#include <cstring>

using namespace eveio;
using namespace eveio::net;

Result<UdpStream> eveio::net::UdpStream::Ipv4Stream() noexcept {
  native_socket_type sock = socket_create(AF_INET, SOCK_DGRAM, 0);
  if (sock == INVALID_NATIVE_SOCKET)
    return Result<UdpStream>::Error(std::strerror(errno));
  return Result<UdpStream>::Ok(UdpStream(sock));
}

Result<UdpStream> eveio::net::UdpStream::Ipv6Stream() noexcept {
  native_socket_type sock = socket_create(AF_INET6, SOCK_DGRAM, 0);
  if (sock == INVALID_NATIVE_SOCKET)
    return Result<UdpStream>::Error(std::strerror(errno));
  return Result<UdpStream>::Ok(UdpStream(sock));
}

eveio::net::UdpStream::UdpStream(UdpStream &&other) noexcept
    : sock(other.sock) {
  other.sock = INVALID_NATIVE_SOCKET;
}

UdpStream &eveio::net::UdpStream::operator=(UdpStream &&other) noexcept {
  if (sock != INVALID_NATIVE_SOCKET)
    socket_close(sock);
  sock = other.sock;
  other.sock = INVALID_NATIVE_SOCKET;
  return (*this);
}

eveio::net::UdpStream::~UdpStream() noexcept {
  if (sock != INVALID_NATIVE_SOCKET)
    socket_close(sock);
}

int64_t eveio::net::UdpStream::SendTo(StringRef data,
                                      const InetAddr &target) const noexcept {
  return SendTo(data.data(), data.size(), target);
}

int64_t eveio::net::UdpStream::SendTo(const void *data,
                                      size_t size,
                                      const InetAddr &target) const noexcept {
  return socket_sendto(sock,
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
  return socket_recvfrom(sock, buf, cap, peer_addr, &len);
}

bool eveio::net::UdpStream::SetNonblock(bool on) const noexcept {
  return socket_set_nonblock(sock, on);
}
