#include "eveio/net/TcpSocket.hpp"

#include "eveio/Result.hpp"
#include "eveio/net/Socket.hpp"
#include "eveio/net/TcpConnection.hpp"

#include <spdlog/spdlog.h>

#include <cerrno>
#include <cstring>

using namespace eveio;
using namespace eveio::net;

Result<TcpSocket> eveio::net::TcpSocket::Create(const InetAddr &addr) noexcept {
  native_socket_type sock = socket_create(addr.GetFamily(), SOCK_STREAM, 0);
  if (sock == INVALID_NATIVE_SOCKET)
    return Result<TcpSocket>::Error(std::strerror(errno));

  if (!socket_bind(
          sock, addr.AsSockaddr(), static_cast<socklen_t>(addr.Size()))) {
    int saved_errno = errno;
    socket_close(sock);
    return Result<TcpSocket>::Error(std::strerror(saved_errno));
  }

  return Result<TcpSocket>::Ok(TcpSocket(sock, addr));
}

bool eveio::net::TcpSocket::Listen(int n) const noexcept {
  return socket_listen(sock, n);
}

void eveio::net::TcpSocket::CloseWrite() const noexcept {
  if (!socket_close_tcp_write(sock))
    SPDLOG_ERROR("failed to close tcp write for socket {}: {}.",
                 sock,
                 std::strerror(errno));
}

Result<TcpConnection> eveio::net::TcpSocket::Accept() const noexcept {
  struct sockaddr_in6 addr {};
  socklen_t len = sizeof(addr);

  native_socket_type conn_sock =
      socket_accept(sock, reinterpret_cast<struct sockaddr *>(&addr), &len);
  if (conn_sock == INVALID_NATIVE_SOCKET)
    return Result<TcpConnection>::Error(std::strerror(errno));

  return Result<TcpConnection>::Ok(TcpConnection(conn_sock, addr));
}
