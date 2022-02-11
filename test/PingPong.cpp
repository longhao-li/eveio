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

#include "eveio/eveio.hpp"

#include <chrono>
#include <cstdio>
#include <memory>

using namespace eveio;

class EchoServer {
  TcpServer server;

public:
  EchoServer(EventLoop &loop, const InetAddress &addr) : server(loop, addr) {
    server.SetMessageCallback(
        [this](AsyncTcpConnection *conn,
               AsyncTcpConnectionBuffer &input,
               std::chrono::system_clock::time_point time) {
          this->OnMessage(conn, input, time);
        });
    server.Start();
  }

  void OnMessage(AsyncTcpConnection *conn,
                 AsyncTcpConnectionBuffer &input,
                 std::chrono::system_clock::time_point) noexcept {
    conn->AsyncSend(input.RetrieveAsString());
  }
};

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s port\n", argv[0]);
    return -10;
  }

  int port = atoi(argv[1]);

  try {
    EventLoop loop;
    InetAddress addr = InetAddress::Ipv4Any(static_cast<uint16_t>(port));
    EchoServer server(loop, addr);
    loop.Loop();
  } catch (const Exception &e) {
    printf("%s\n", e.what());
  }

  return 0;
}
