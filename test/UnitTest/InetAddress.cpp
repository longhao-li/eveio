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

#include <gtest/gtest.h>

using eveio::InetAddress;
using eveio::String;
using eveio::StringRef;

TEST(eveioInetAddress, ipv4) {
  InetAddress addr0 = InetAddress::Ipv4Any(1234);
  ASSERT_EQ(addr0.GetIP(), String("0.0.0.0"));
  ASSERT_EQ(addr0.GetIPWithPort(), String("0.0.0.0:1234"));
  ASSERT_EQ(addr0.GetPort(), 1234);
  ASSERT_TRUE(addr0.IsIpv4());
  ASSERT_FALSE(addr0.IsIpv6());

  ASSERT_NO_THROW(InetAddress("0.0.0.0", 1234));

  InetAddress addr1 = InetAddress::Ipv4Loopback(4321);
  ASSERT_EQ(addr1.GetIP(), String("127.0.0.1"));
  ASSERT_EQ(addr1.GetIPWithPort(), String("127.0.0.1:4321"));
  ASSERT_EQ(addr1.GetPort(), 4321);
  ASSERT_TRUE(addr1.IsIpv4());
  ASSERT_FALSE(addr1.IsIpv6());

  ASSERT_NO_THROW(InetAddress("127.0.0.1", 4321));

  ASSERT_NO_THROW(InetAddress("1.2.3.4", 8888));
  InetAddress addr2("1.2.3.4", 8888);
  ASSERT_EQ(addr2.GetIP(), String("1.2.3.4"));
  ASSERT_EQ(addr2.GetIPWithPort(), String("1.2.3.4:8888"));
  ASSERT_EQ(addr2.GetPort(), 8888);

  ASSERT_NO_THROW(InetAddress("255.254.253.252", 65535));
  InetAddress addr3("255.254.253.252", 65535);
  ASSERT_EQ(addr3.GetIP(), String("255.254.253.252"));
  ASSERT_EQ(addr3.GetIPWithPort(), String("255.254.253.252:65535"));
  ASSERT_EQ(addr3.GetPort(), 65535);

  ASSERT_NE(addr0, addr1);
  ASSERT_NE(addr0, addr2);
  ASSERT_NE(addr0, addr3);
  ASSERT_NE(addr1, addr2);
  ASSERT_NE(addr1, addr3);
  ASSERT_NE(addr2, addr3);

  ASSERT_THROW(InetAddress("--::--", 5678), eveio::InvalidParameterException);
}

TEST(eveioInetAddress, ipv6) {
  auto addr0 = InetAddress::Ipv6Any(1234);
  ASSERT_EQ(addr0.GetIP(), String("::"));
  ASSERT_EQ(addr0.GetIPWithPort(), String("[::]:1234"));
  ASSERT_EQ(addr0.GetPort(), 1234);
  ASSERT_FALSE(addr0.IsIpv4());
  ASSERT_TRUE(addr0.IsIpv6());

  ASSERT_NO_THROW(InetAddress("0:0:0:0:0:0:0:0", 1234));
}
