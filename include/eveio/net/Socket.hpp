#ifndef EVEIO_NET_SOCKET_HPP
#define EVEIO_NET_SOCKET_HPP

#include "eveio/net/Config.hpp"
#include <sys/socket.h>

namespace eveio {
namespace net {
namespace detail {
#if defined(EVEIO_OS_WIN32)
typedef SOCKET native_socket_type;
#else
typedef int native_socket_type;

static constexpr native_socket_type InvalidSocket = -1;

inline native_socket_type socket(int domain, int type, int proto) noexcept {
  native_socket_type sock = ::socket(domain, type, proto);
  if (sock < 0)
    return InvalidSocket;
  else
    return sock;
}

inline bool bind(native_socket_type sock,
                 const struct sockaddr *addr,
                 socklen_t len) noexcept {
  if (::bind(sock, addr, len) < 0)
    return false;
  return true;
}

inline bool listen(native_socket_type sock, int n) noexcept {
  return (::listen(sock, n) >= 0);
}

inline native_socket_type accept(native_socket_type sock,
                                 struct sockaddr *addr,
                                 socklen_t *len) noexcept {
  native_socket_type res = ::accept(sock, addr, len);
  if (res < 0)
    return InvalidSocket;
  else
    return res;
}

inline bool connect(native_socket_type sock,
                    struct sockaddr *addr,
                    socklen_t len) noexcept {
  if (::connect(sock, addr, len) <= 0)
    return false;
  return true;
}

inline bool close_socket(native_socket_type socket) noexcept {
  return (::close(socket) == 0);
}

inline bool set_nonblock(native_socket_type sock, bool on) noexcept {
  int opt = ::fcntl(sock, F_GETFL);
  if (on)
    opt |= O_NONBLOCK;
  else
    opt &= ~O_NONBLOCK;
  return (::fcntl(sock, F_SETFL, opt) >= 0);
}

inline bool set_tcp_nodelay(native_socket_type sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) >= 0);
}

inline bool set_reuseaddr(native_socket_type sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) >= 0);
}

inline bool set_reuseport(native_socket_type sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) >= 0);
}

inline bool set_keepalive(native_socket_type sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) >= 0);
}

inline bool close_tcp_write(native_socket_type sock) noexcept {
  return (::shutdown(sock, SHUT_WR) >= 0);
}

inline int
socket_read(native_socket_type sock, void *buf, size_t cap) noexcept {
  return ::read(sock, buf, cap);
}

inline int
socket_write(native_socket_type sock, const void *buf, size_t size) noexcept {
  return ::write(sock, buf, size);
}

#endif
} // namespace detail
} // namespace net
} // namespace eveio

#endif // EVEIO_NET_SOCKET_HPP
