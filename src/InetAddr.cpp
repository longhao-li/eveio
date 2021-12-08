#include "eveio/net/InetAddr.hpp"
#include "eveio/Result.hpp"
#include "eveio/String.hpp"

#include "eveio/Format.hpp"

#include <cassert>
#include <cstring>
#include <memory>

using namespace eveio;
using namespace eveio::net;

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

eveio::net::InetAddr::InetAddr(const struct sockaddr_in &addr) noexcept
    : addr4(addr) {}

eveio::net::InetAddr::InetAddr(const struct sockaddr_in6 &addr) noexcept
    : addr6(addr) {}

Result<InetAddr> eveio::net::InetAddr::Create(StringRef ip,
                                              uint16_t port) noexcept {
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
    return Result<InetAddr>::Error("invalid ip address.");
  return Result<InetAddr>::Ok(addr);
}

String eveio::net::InetAddr::GetIp() const noexcept {
  char buf[128]{};
  if (IsIpv4())
    ::inet_ntop(GetFamily(), std::addressof(addr4.sin_addr), buf, sizeof(buf));
  else
    ::inet_ntop(GetFamily(), std::addressof(addr6.sin6_addr), buf, sizeof(buf));
  return String(buf);
}

String eveio::net::InetAddr::GetIpWithPort() const noexcept {
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

InetAddr eveio::net::InetAddr::Ipv4Loopback(uint16_t port) noexcept {
  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));
  addr.addr4.sin_family = AF_INET;
  addr.addr4.sin_port = htons(port);
  addr.addr4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  return InetAddr(addr);
}

InetAddr eveio::net::InetAddr::Ipv4Any(uint16_t port) noexcept {
  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));
  addr.addr4.sin_family = AF_INET;
  addr.addr4.sin_port = htons(port);
  addr.addr4.sin_addr.s_addr = htonl(INADDR_ANY);
  return InetAddr(addr);
}

InetAddr eveio::net::InetAddr::Ipv6Loopback(uint16_t port) noexcept {
  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));
  addr.addr6.sin6_family = AF_INET6;
  addr.addr6.sin6_port = htons(port);
  addr.addr6.sin6_addr = in6addr_loopback;
  return InetAddr(addr);
}

InetAddr eveio::net::InetAddr::Ipv6Any(uint16_t port) noexcept {
  InetAddr addr;
  std::memset(&addr.addr6, 0, sizeof(addr.addr6));
  addr.addr6.sin6_family = AF_INET6;
  addr.addr6.sin6_port = htons(port);
  addr.addr6.sin6_addr = in6addr_any;
  return InetAddr(addr);
}
