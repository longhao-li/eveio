#include "eveio/net/InetAddr.hpp"
#include "eveio/net/UdpStream.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace eveio;

static char buf[1024];

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s [port].\n", argv[0]);
    return 1;
  }

  int port = atoi(argv[1]);
  auto stream_res = net::UdpStream::Ipv4Stream();
  assert(stream_res.IsValid());
  auto &stream = stream_res.Unwarp();
  auto peer = net::InetAddr::Ipv4Loopback(port);

  String str;
  for (;;) {
    str.clear();
    std::getline(std::cin, str);
    stream.SendTo(str, peer);
    int len = stream.ReceiveFrom(buf, sizeof(buf), peer);
    assert(len >= 0);
    buf[len] = 0;
    printf("%s\n", buf);
  }
  return 0;
}
