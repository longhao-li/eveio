/// Copyright (c) 2021 Li Longhao
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.

#ifndef EVEIO_INET_ADDRESS_HPP
#define EVEIO_INET_ADDRESS_HPP

#include "eveio/Config.hpp"
#include "eveio/String.hpp"

#include <cstdint>

namespace eveio {

class InetAddress {
  union {
    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;
  };

public:
  /// Throw InvalidParameterException if ip is invalid.
  InetAddress(const String &ip, uint16_t port);

  InetAddress(const InetAddress &) = default;
  InetAddress &operator=(const InetAddress &) = default;

  InetAddress(InetAddress &&) = default;
  InetAddress &operator=(InetAddress &&) = default;

  ~InetAddress() noexcept = default;

  InetAddress(const struct sockaddr_in &addr) noexcept;
  InetAddress(const struct sockaddr_in6 &addr) noexcept;

  String GetIP() const noexcept;
  String GetIPWithPort() const noexcept;

  uint16_t GetPort() const noexcept;
  void SetPort(uint16_t port) noexcept;

  sa_family_t GetFamily() const noexcept { return addr4.sin_family; }
  bool IsIpv4() const noexcept { return GetFamily() == AF_INET; }
  bool IsIpv6() const noexcept { return GetFamily() == AF_INET6; }

  void SetIpv6ScopeID(uint32_t scopeID) noexcept;

  size_t size() const noexcept {
    return IsIpv4() ? sizeof(addr4) : sizeof(addr6);
  }

  const struct sockaddr *AsSockaddr() const noexcept {
    return reinterpret_cast<const struct sockaddr *>(&addr6);
  }

  bool operator==(const InetAddress &rhs) const noexcept;
  bool operator!=(const InetAddress &rhs) const noexcept {
    return !(*this == rhs);
  }

  static InetAddress Ipv4Loopback(uint16_t port) noexcept;
  static InetAddress Ipv4Any(uint16_t port) noexcept;
  static InetAddress Ipv6Loopback(uint16_t port) noexcept;
  static InetAddress Ipv6Any(uint16_t port) noexcept;
};

} // namespace eveio

#endif // EVEIO_INET_ADDRESS_HPP
