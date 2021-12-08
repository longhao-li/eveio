#ifndef EVEIO_NET_SOCKET_HPP
#define EVEIO_NET_SOCKET_HPP

#include "eveio/net/Config.hpp"
#include "eveio/net/InetAddr.hpp"
#include <sys/socket.h>

namespace eveio {
namespace net {
namespace detail {
#if defined(EVEIO_OS_WIN32)
typedef SOCKET native_socket_type;
#elif defined(EVEIO_OS_DARWIN)

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

inline bool set_no_sigpipe(native_socket_type sock, bool on) noexcept {
  int opt = on ? 1 : 0;
  return (::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt)) >= 0);
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

inline int socket_sendto(native_socket_type sock,
                         const void *buf,
                         size_t size,
                         const struct sockaddr *addr,
                         socklen_t len) noexcept {
  return ::sendto(sock, buf, size, MSG_NOSIGNAL, addr, len);
}

inline int socket_recvfrom(native_socket_type sock,
                           void *buf,
                           size_t cap,
                           struct sockaddr *addr,
                           socklen_t *len) noexcept {
  return ::recvfrom(sock, buf, cap, 0, addr, len);
}

#elif defined(EVEIO_OS_LINUX)
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

/// NOTE use MSG_NOSIGPIPE in send
inline bool set_no_sigpipe(native_socket_type, bool) noexcept { return true; }

inline bool close_tcp_write(native_socket_type sock) noexcept {
  return (::shutdown(sock, SHUT_WR) >= 0);
}

inline int
socket_read(native_socket_type sock, void *buf, size_t cap) noexcept {
  return ::recv(sock, buf, cap, MSG_NOSIGNAL);
}

inline int
socket_write(native_socket_type sock, const void *buf, size_t size) noexcept {
  return ::send(sock, buf, size, MSG_NOSIGNAL);
}
#endif

class SocketBase {
public:
  typedef detail::native_socket_type native_socket_type;
  static constexpr native_socket_type InvalidSocket = detail::InvalidSocket;

protected:
  native_socket_type sock;
  InetAddr local_addr;

  SocketBase(native_socket_type h, const InetAddr &addr) noexcept
      : sock(h), local_addr(addr) {}

  // disable upcast
  ~SocketBase() noexcept;

public:
  SocketBase(const SocketBase &) = delete;
  SocketBase &operator=(const SocketBase &) = delete;

  SocketBase(SocketBase &&other) noexcept;
  SocketBase &operator=(SocketBase &&other) noexcept;

  bool SetNonblock(bool on) const noexcept;
  bool SetReuseAddr(bool on) const noexcept;
  bool SetReusePort(bool on) const noexcept;

  const InetAddr &LocalAddr() const noexcept { return local_addr; }

  native_socket_type native_socket() const noexcept { return sock; }
  operator native_socket_type() const noexcept { return sock; }
};

} // namespace detail
} // namespace net
} // namespace eveio

#endif // EVEIO_NET_SOCKET_HPP
