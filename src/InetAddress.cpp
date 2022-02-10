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

#include "eveio/InetAddress.hpp"
#include "eveio/Exception.hpp"
#include "eveio/String.hpp"

#include <cassert>
#include <cstring>

using namespace eveio;

eveio::InetAddress::InetAddress(const String &ip, uint16_t port) : addr6{} {
  addr4.sin_family = AF_INET;
  for (const char c : ip) {
    if (c == ':') {
      addr6.sin6_family = AF_INET6;
      break;
    }
  }

  SetPort(port);
  int res = 0;
  if (IsIpv4()) {
    res = ::inet_pton(GetFamily(), ip.c_str(), std::addressof(addr4.sin_addr));
  } else {
    assert(IsIpv6());
    res = ::inet_pton(GetFamily(), ip.c_str(), std::addressof(addr6.sin6_addr));
  }

  if (res != 1) {
    throw InvalidParameterException(
        __FILENAME__, __LINE__, __func__, "Invalid IP address: " + ip);
  }
}

eveio::InetAddress::InetAddress(const struct sockaddr_in &addr) noexcept
    : addr4(addr) {}

eveio::InetAddress::InetAddress(const struct sockaddr_in6 &addr) noexcept
    : addr6(addr) {}

String eveio::InetAddress::GetIP() const noexcept {
  char buf[128]{};
  if (IsIpv4()) {
    ::inet_ntop(GetFamily(), std::addressof(addr4.sin_addr), buf, sizeof(buf));
  } else {
    ::inet_ntop(GetFamily(), std::addressof(addr6.sin6_addr), buf, sizeof(buf));
  }
  return {buf};
}

String eveio::InetAddress::GetIPWithPort() const noexcept {
  if (IsIpv4()) {
    return GetIP() + ":" + std::to_string(GetPort());
  } else {
    return "[" + GetIP() + "]:" + std::to_string(GetPort());
  }
}

uint16_t eveio::InetAddress::GetPort() const noexcept {
  if (IsIpv4()) {
    return ntohs(addr4.sin_port);
  } else {
    return ntohs(addr6.sin6_port);
  }
}

void eveio::InetAddress::SetPort(uint16_t port) noexcept {
  if (IsIpv4()) {
    addr4.sin_port = htons(port);
  } else {
    addr6.sin6_port = htons(port);
  }
}

void eveio::InetAddress::SetIpv6ScopeID(uint32_t scopeID) noexcept {
  if (IsIpv6()) {
    addr6.sin6_scope_id = htonl(scopeID);
  }
}

bool eveio::InetAddress::operator==(const InetAddress &rhs) const noexcept {
  if (this == &rhs) {
    return true;
  }

  if (GetFamily() != rhs.GetFamily()) {
    return false;
  }

  if (IsIpv4()) {
    return (memcmp(&addr4, &rhs.addr4, sizeof(addr4)) == 0);
  } else {
    return (memcmp(&addr6, &rhs.addr6, sizeof(addr6)) == 0);
  }
}

InetAddress eveio::InetAddress::Ipv4Loopback(uint16_t port) noexcept {
  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  return {addr};
}

InetAddress eveio::InetAddress::Ipv4Any(uint16_t port) noexcept {
  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  return {addr};
}

InetAddress eveio::InetAddress::Ipv6Loopback(uint16_t port) noexcept {
  struct sockaddr_in6 addr {};
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  addr.sin6_addr = in6addr_loopback;
  return {addr};
}

InetAddress eveio::InetAddress::Ipv6Any(uint16_t port) noexcept {
  struct sockaddr_in6 addr {};
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  addr.sin6_addr = in6addr_any;
  return {addr};
}
