#ifndef EVEIO_NET_INETADDR_HPP
#define EVEIO_NET_INETADDR_HPP

#include "eveio/Result.hpp"
#include "eveio/String.hpp"
#include "eveio/net/Config.hpp"

#include <cstdint>

namespace eveio {
namespace net {

class InetAddr {
  union {
    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;
  };

  InetAddr() noexcept = default;

public:
  InetAddr(const struct sockaddr_in &addr) noexcept;
  InetAddr(const struct sockaddr_in6 &addr) noexcept;

  InetAddr(const InetAddr &) noexcept = default;
  InetAddr &operator=(const InetAddr &) noexcept = default;
  ~InetAddr() noexcept = default;

  static Result<InetAddr, const char *> Create(StringRef ip,
                                               uint16_t port) noexcept;

  String GetIp() const noexcept;
  String GetIpWithPort() const noexcept;

  uint16_t GetPort() const noexcept;
  void SetPort(uint16_t port) noexcept;
  void SetIpv6ScopeID(uint32_t scope_id) noexcept;

  uint8_t GetFamily() const noexcept { return addr4.sin_family; }
  bool IsIpv4() const noexcept { return GetFamily() == AF_INET; }
  bool IsIpv6() const noexcept { return GetFamily() == AF_INET6; }

  size_t Size() const noexcept {
    return IsIpv4() ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
  }

  const struct sockaddr *AsSockaddr() const noexcept {
    return reinterpret_cast<const struct sockaddr *>(&addr6);
  }

  bool operator==(const InetAddr &rhs) const noexcept;
  bool operator!=(const InetAddr &rhs) const noexcept {
    return !((*this) == rhs);
  }

  static InetAddr Ipv4Loopback(uint16_t port) noexcept;
  static InetAddr Ipv4Any(uint16_t port) noexcept;
  static InetAddr Ipv6Loopback(uint16_t port) noexcept;
  static InetAddr Ipv6Any(uint16_t port) noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_INETADDR_HPP
