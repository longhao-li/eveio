#include "eveio/InetAddr.h"

#include <cstring>

using namespace eveio;

eveio::InetAddr::InetAddr(std::string ip, uint16_t port) noexcept {
    m_addr4.sin_family = AF_INET;
    for (const char c : ip) {
        if (c == ':') {
            m_addr6.sin6_family = AF_INET6;
            break;
        }
    }

    SetPort(port);
    int res = 0;
    if (IsIpv4()) {
        res = ::inet_pton(
            GetFamily(), ip.c_str(), std::addressof(m_addr4.sin_addr));
    } else {
        res = ::inet_pton(
            GetFamily(), ip.c_str(), std::addressof(m_addr6.sin6_addr));
    }

    if (res != 1) {
        // Invalid address.
        m_addr4.sin_family = 0;
    }
}

eveio::InetAddr::InetAddr(const struct sockaddr_in &addr) noexcept
    : m_addr4(addr) {}

eveio::InetAddr::InetAddr(const struct sockaddr_in6 &addr) noexcept
    : m_addr6(addr) {}

std::string eveio::InetAddr::GetIP() const noexcept {
    char buf[128]{};
    if (IsIpv4()) {
        ::inet_ntop(
            GetFamily(), std::addressof(m_addr4.sin_addr), buf, sizeof(buf));
    } else {
        ::inet_ntop(
            GetFamily(), std::addressof(m_addr6.sin6_addr), buf, sizeof(buf));
    }
    return {buf};
}

std::string eveio::InetAddr::GetIpWithPort() const noexcept {
    if (IsIpv4()) {
        return GetIP() + ':' + std::to_string(GetPort());
    } else {
        return '[' + GetIP() + "]:" + std::to_string(GetPort());
    }
}

uint16_t eveio::InetAddr::GetPort() const noexcept {
    if (IsIpv4()) {
        return ntohs(m_addr4.sin_port);
    } else {
        return ntohs(m_addr6.sin6_port);
    }
}

void eveio::InetAddr::SetPort(uint16_t port) noexcept {
    if (IsIpv4()) {
        m_addr4.sin_port = htons(port);
    } else {
        m_addr6.sin6_port = htons(port);
    }
}

void eveio::InetAddr::SetIpv6ScopeID(uint32_t scopeID) noexcept {
    if (IsIpv6()) {
        m_addr6.sin6_scope_id = htonl(scopeID);
    }
}

bool eveio::InetAddr::operator==(const InetAddr &rhs) const noexcept {
    if (this == &rhs) {
        return true;
    }

    if (GetFamily() != rhs.GetFamily()) {
        return false;
    }

    if (IsIpv4()) {
        return (memcmp(&m_addr4, &rhs.m_addr4, sizeof(m_addr4)) == 0);
    } else {
        return (memcmp(&m_addr6, &rhs.m_addr6, sizeof(m_addr6)) == 0);
    }
}

InetAddr eveio::InetAddr::Ipv4Loopback(uint16_t port) noexcept {
    struct sockaddr_in addr {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    return {addr};
}

InetAddr eveio::InetAddr::Ipv4Any(uint16_t port) noexcept {
    struct sockaddr_in addr {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return {addr};
}

InetAddr eveio::InetAddr::Ipv6Loopback(uint16_t port) noexcept {
    struct sockaddr_in6 addr {};
    addr.sin6_family = AF_INET6;
    addr.sin6_port   = htons(port);
    addr.sin6_addr   = in6addr_loopback;
    return {addr};
}

InetAddr eveio::InetAddr::Ipv6Any(uint16_t port) noexcept {
    struct sockaddr_in6 addr {};
    addr.sin6_family = AF_INET6;
    addr.sin6_port   = htons(port);
    addr.sin6_addr   = in6addr_any;
    return {addr};
}
