#pragma once

#include "eveio/Config.h"

#include <string>

namespace eveio {

class InetAddr {
public:
    InetAddr() noexcept = default;
    // Use IsValid() to check if this is valid address.
    InetAddr(std::string ip, uint16_t port) noexcept;

    InetAddr(const struct sockaddr_in &addr4) noexcept;
    InetAddr(const struct sockaddr_in6 &addr6) noexcept;

    InetAddr(const InetAddr &) noexcept = default;
    InetAddr &operator=(const InetAddr &) noexcept = default;

    sa_family_t GetFamily() const noexcept { return m_addr4.sin_family; }

    bool IsIpv4() const noexcept { return GetFamily() == AF_INET; }
    bool IsIpv6() const noexcept { return GetFamily() == AF_INET6; }
    bool IsValid() const noexcept { return (IsIpv4() || IsIpv6()); }

    std::string GetIP() const noexcept;
    std::string GetIpWithPort() const noexcept;

    uint16_t GetPort() const noexcept;
    void     SetPort(uint16_t port) noexcept;

    void SetIpv6ScopeID(uint32_t scopeID) noexcept;

    const struct sockaddr *AsSockaddr() const noexcept {
        return reinterpret_cast<const struct ::sockaddr *>(&m_addr6);
    }

    struct sockaddr *AsSockaddr() noexcept {
        return reinterpret_cast<struct ::sockaddr *>(&m_addr6);
    }

    size_t GetAddrSize() const noexcept {
        return IsIpv4() ? sizeof(m_addr4) : sizeof(m_addr6);
    }

    bool operator==(const InetAddr &rhs) const noexcept;
    bool operator!=(const InetAddr &rhs) const noexcept {
        return !(*this == rhs);
    }

    static InetAddr Ipv4Loopback(uint16_t port) noexcept;
    static InetAddr Ipv4Any(uint16_t port) noexcept;
    static InetAddr Ipv6Loopback(uint16_t port) noexcept;
    static InetAddr Ipv6Any(uint16_t port) noexcept;

private:
    union {
        struct ::sockaddr_in  m_addr4;
        struct ::sockaddr_in6 m_addr6 {};
    };
};

} // namespace eveio
