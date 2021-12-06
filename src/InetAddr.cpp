#include "eveio/net/InetAddr.hpp"
#include "eveio/Result.hpp"
#include "eveio/String.hpp"

#include "eveio/Format.hpp"

#include <cassert>
#include <cstring>
#include <memory>

eveio::net::InetAddr::InetAddr(const struct sockaddr_in &addr) noexcept
    : addr4(addr) {}

eveio::net::InetAddr::InetAddr(const struct sockaddr_in6 &addr) noexcept
    : addr6(addr) {}

eveio::Result<eveio::net::InetAddr, const char *>
eveio::net::InetAddr::Create(StringRef ip, uint16_t port) noexcept {
  String ip_str(ip);

  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));

  addr.addr4.sin_family = AF_INET;
  for (char c : ip_str) {
    if (c == ':') {
      addr.addr6.sin6_family = AF_INET6;
      break;
    }
  }

  addr.SetPort(port);
  int res = 0;

  if (addr.IsIpv4())
    res = ::inet_pton(
        addr.GetFamily(), ip_str.c_str(), std::addressof(addr.addr4.sin_addr));
  else {
    assert(addr.IsIpv6());
    res = ::inet_pton(
        addr.GetFamily(), ip_str.c_str(), std::addressof(addr.addr6.sin6_addr));
  }

  if (res != 1)
    return Result<InetAddr, const char *>::Error("invalid ip address.");
  return Result<InetAddr, const char *>::Ok(addr);
}

eveio::String eveio::net::InetAddr::GetIp() const noexcept {
  char buf[128]{};
  if (IsIpv4())
    ::inet_ntop(GetFamily(), std::addressof(addr4.sin_addr), buf, sizeof(buf));
  else
    ::inet_ntop(GetFamily(), std::addressof(addr6.sin6_addr), buf, sizeof(buf));
  return String(buf);
}

eveio::String eveio::net::InetAddr::GetIpWithPort() const noexcept {
  if (IsIpv4())
    return String(Format("{}:{}", GetIp(), GetPort()));
  else
    return String(Format("[{}]:{}", GetIp(), GetPort()));
}

uint16_t eveio::net::InetAddr::GetPort() const noexcept {
  if (IsIpv4())
    return ntohs(addr4.sin_port);
  else
    return ntohs(addr6.sin6_port);
}

void eveio::net::InetAddr::SetPort(uint16_t port) noexcept {
  if (IsIpv4())
    addr4.sin_port = htons(port);
  else
    addr6.sin6_port = htons(port);
}

void eveio::net::InetAddr::SetIpv6ScopeID(uint32_t scope_id) noexcept {
  if (IsIpv6())
    addr6.sin6_scope_id = htonl(scope_id);
}

bool eveio::net::InetAddr::operator==(const InetAddr &rhs) const noexcept {
  if (GetFamily() == rhs.GetFamily()) {
    if (IsIpv4())
      return (std::memcmp(&addr4, &rhs.addr4, sizeof(addr4)) == 0);
    else
      return (std::memcmp(&addr6, &rhs.addr6, sizeof(addr6)) == 0);
  }
  return false;
}

eveio::net::InetAddr
eveio::net::InetAddr::Ipv4Loopback(uint16_t port) noexcept {
  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));
  addr.addr4.sin_family = AF_INET;
  addr.addr4.sin_port = htons(port);
  addr.addr4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  return InetAddr(addr);
}

eveio::net::InetAddr eveio::net::InetAddr::Ipv4Any(uint16_t port) noexcept {
  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));
  addr.addr4.sin_family = AF_INET;
  addr.addr4.sin_port = htons(port);
  addr.addr4.sin_addr.s_addr = htonl(INADDR_ANY);
  return InetAddr(addr);
}

eveio::net::InetAddr
eveio::net::InetAddr::Ipv6Loopback(uint16_t port) noexcept {
  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));
  addr.addr6.sin6_family = AF_INET6;
  addr.addr6.sin6_port = htons(port);
  addr.addr6.sin6_addr = in6addr_loopback;
  return InetAddr(addr);
}

eveio::net::InetAddr eveio::net::InetAddr::Ipv6Any(uint16_t port) noexcept {
  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));
  addr.addr6.sin6_family = AF_INET6;
  addr.addr6.sin6_port = htons(port);
  addr.addr6.sin6_addr = in6addr_any;
  return InetAddr(addr);
}
