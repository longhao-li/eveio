#include "eveio/net/Buffer.hpp"
#include "eveio/String.hpp"

#include <gtest/gtest.h>

using namespace eveio;
using namespace eveio::net;

TEST(eveio_Buffer, AppendRetrive) {
  Buffer buf;
  ASSERT_EQ(buf.Size(), 0);

  String str(200, 'x');
  buf.Append(str.data(), str.size());
  ASSERT_EQ(buf.Size(), str.size());

  String str2 = buf.RetrieveAsString();
  ASSERT_EQ(str2, str);
  ASSERT_TRUE(buf.IsEmpty());

  buf.Append(str);
  buf.Append(str2);
  ASSERT_EQ(buf.Size(), str.size() * 2);
  ASSERT_EQ(buf.Capacity(), 0);

  buf.Readout(100);
  ASSERT_EQ(buf.Size(), 300);
  ASSERT_EQ(buf.Capacity(), 0);
  buf.Readout(300);
  ASSERT_EQ(buf.Size(), 0);
  ASSERT_EQ(buf.Capacity(), 400);
}
