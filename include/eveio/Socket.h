#pragma once

#include "eveio/Config.h"
#include "eveio/InetAddr.h"

namespace eveio {

#if EVEIO_OS_WIN32
#else
using socket_t                          = int;
constexpr const socket_t INVALID_SOCKET = -1;

namespace socket {
inline socket_t create(int domain, int type, int proto) noexcept {
    socket_t sock = ::socket(domain, type, proto);
    if (sock < 0) {
        return INVALID_SOCKET;
    } else {
        return sock;
    }
}

inline bool bind(socket_t sock, const struct sockaddr *addr,
                 size_t len) noexcept {
    if (::bind(sock, addr, static_cast<socklen_t>(len)) < 0) {
        return false;
    }
    return true;
}

inline bool listen(socket_t sock, int n) noexcept {
    return (::listen(sock, n) >= 0);
}

inline socket_t accept(socket_t sock, struct sockaddr *addr,
                       size_t *len) noexcept {
    auto     tempLen = static_cast<socklen_t>(*len);
    socket_t res     = ::accept(sock, addr, &tempLen);
    if (res < 0) {
        return INVALID_SOCKET;
    }

    *len = static_cast<size_t>(tempLen);
    return res;
}

inline bool connect(socket_t sock, const struct sockaddr *addr,
                    size_t len) noexcept {
    return (::connect(sock, addr, static_cast<socklen_t>(len)) >= 0);
}

inline bool close(socket_t sock) noexcept { return (::close(sock) == 0); }

inline bool setnonblock(socket_t sock, bool on) noexcept {
    int opt = ::fcntl(sock, F_GETFL);
    if (on) {
        opt |= O_NONBLOCK;
    } else {
        opt &= ~O_NONBLOCK;
    }
    return (::fcntl(sock, F_SETFL, opt) >= 0);
}

inline bool settcpnodelay(socket_t sock, bool on) noexcept {
    int opt = on ? 1 : 0;
    return ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) >= 0;
}

inline bool setreuseaddr(socket_t sock, bool on) noexcept {
    int opt = on ? 1 : 0;
    return ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) >= 0;
}

inline bool setreuseport(socket_t sock, bool on) noexcept {
    int opt = on ? 1 : 0;
    return ::setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) >= 0;
}

inline bool setkeepalive(socket_t sock, bool on) noexcept {
    int opt = on ? 1 : 0;
    return ::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) >= 0;
}

inline bool shutdown_write(socket_t sock) noexcept {
    return (::shutdown(sock, SHUT_WR) >= 0);
}

inline int64_t read(socket_t sock, void *buffer, size_t cap) noexcept {
    return ::recv(sock, buffer, cap, 0);
}

inline int64_t write(socket_t sock, const void *data, size_t size) noexcept {
    return ::send(sock, data, size, MSG_NOSIGNAL);
}

inline int64_t recvfrom(socket_t sock, void *buffer, size_t cap,
                        struct sockaddr *addr, size_t *len) noexcept {
    auto tempLen = static_cast<socklen_t>(*len);
    auto res     = ::recvfrom(sock, buffer, cap, 0, addr, &tempLen);
    if (res < 0) {
        return -1;
    }
    *len = static_cast<size_t>(tempLen);
    return res;
}

inline int64_t sendto(socket_t sock, const void *buf, size_t size,
                      const struct sockaddr *addr, size_t addrLen) noexcept {
    return ::sendto(
        sock, buf, size, MSG_NOSIGNAL, addr, static_cast<socklen_t>(addrLen));
}

inline bool getpeername(socket_t sock, InetAddr &addr) noexcept {
    socklen_t sock_len = sizeof(struct sockaddr_in6);
    return ::getpeername(sock, addr.AsSockaddr(), &sock_len) == 0;
}

} // namespace socket
#endif
} // namespace eveio
