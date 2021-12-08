#ifndef EVEIO_NET_INETADDR_HPP
#define EVEIO_NET_INETADDR_HPP

#include "eveio/Result.hpp"
#include "eveio/String.hpp"
#include "eveio/net/Config.hpp"

#include <cstdint>

/// Muduo - A reactor-based C++ network library for Linux
/// Copyright (c) 2010, Shuo Chen.  All rights reserved.
/// http://code.google.com/p/muduo/
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions
/// are met:
///
///   * Redistributions of source code must retain the above copyright
/// notice, this list of conditions and the following disclaimer.
///   * Redistributions in binary form must reproduce the above copyright
/// notice, this list of conditions and the following disclaimer in the
/// documentation and/or other materials provided with the distribution.
///   * Neither the name of Shuo Chen nor the names of other contributors
/// may be used to endorse or promote products derived from this software
/// without specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
/// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
/// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
/// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
/// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

  static Result<InetAddr> Create(StringRef ip, uint16_t port) noexcept;

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
