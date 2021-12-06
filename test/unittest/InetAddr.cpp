#include "eveio/String.hpp"
#include "eveio/net/InetAddr.hpp"

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

using eveio::String;
using eveio::StringRef;
using eveio::net::InetAddr;

TEST(eveio_InetAddr, ipv4) {
  InetAddr addr0 = InetAddr::Ipv4Any(1234);
  ASSERT_EQ(addr0.GetIp(), String("0.0.0.0"));
  ASSERT_EQ(addr0.GetIpWithPort(), String("0.0.0.0:1234"));
  ASSERT_EQ(addr0.GetPort(), 1234);
  ASSERT_TRUE(addr0.IsIpv4());
  ASSERT_FALSE(addr0.IsIpv6());

  auto r_addr0 = InetAddr::Create("0.0.0.0", 1234);
  ASSERT_TRUE(r_addr0.IsValid());
  ASSERT_EQ(r_addr0.GetValue(), addr0);

  InetAddr addr1 = InetAddr::Ipv4Loopback(4321);
  ASSERT_EQ(addr1.GetIp(), String("127.0.0.1"));
  ASSERT_EQ(addr1.GetIpWithPort(), String("127.0.0.1:4321"));
  ASSERT_EQ(addr1.GetPort(), 4321);
  ASSERT_TRUE(addr1.IsIpv4());
  ASSERT_FALSE(addr1.IsIpv6());

  auto r_addr1 = InetAddr::Create("127.0.0.1", 4321);
  ASSERT_TRUE(r_addr1.IsValid());
  ASSERT_EQ(r_addr1.GetValue(), addr1);

  auto r_addr2 = InetAddr::Create("1.2.3.4", 8888);
  ASSERT_TRUE(r_addr2.IsValid());
  InetAddr &addr2 = r_addr2.GetValue();
  ASSERT_EQ(addr2.GetIp(), String("1.2.3.4"));
  ASSERT_EQ(addr2.GetIpWithPort(), String("1.2.3.4:8888"));
  ASSERT_EQ(addr2.GetPort(), 8888);

  auto r_addr3 = InetAddr::Create("255.254.253.252", 65535);
  ASSERT_TRUE(r_addr3.IsValid());
  InetAddr &addr3 = r_addr3.GetValue();
  ASSERT_EQ(addr3.GetIp(), String("255.254.253.252"));
  ASSERT_EQ(addr3.GetIpWithPort(), String("255.254.253.252:65535"));
  ASSERT_EQ(addr3.GetPort(), 65535);

  ASSERT_NE(addr0, addr1);
  ASSERT_NE(addr0, addr2);
  ASSERT_NE(addr0, addr3);
  ASSERT_NE(addr1, addr2);
  ASSERT_NE(addr1, addr3);
  ASSERT_NE(addr2, addr3);

  auto er_addr4 = InetAddr::Create("--::--", 5678);
  ASSERT_FALSE(er_addr4.IsValid());
  SPDLOG_INFO("error info: {}", er_addr4.GetError());
}

TEST(eveio_InetAddr, ipv6) {
  InetAddr addr0 = InetAddr::Ipv6Any(1234);
  ASSERT_EQ(addr0.GetIp(), String("::"));
  ASSERT_EQ(addr0.GetIpWithPort(), String("[::]:1234"));
  ASSERT_EQ(addr0.GetPort(), 1234);
  ASSERT_FALSE(addr0.IsIpv4());
  ASSERT_TRUE(addr0.IsIpv6());

  auto r_addr0 = InetAddr::Create("0:0:0:0:0:0:0:0", 1234);
  ASSERT_TRUE(r_addr0.IsValid());
  ASSERT_EQ(r_addr0.GetValue(), addr0);
}
