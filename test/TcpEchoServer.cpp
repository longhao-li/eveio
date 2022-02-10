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

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <thread>

using namespace eveio;

class EchoServer {
  TcpServer server;

public:
  EchoServer(Eventloop &loop, const InetAddress &addr) : server(loop, addr) {
    server.SetConnectionCallback([this](AsyncTcpConnection *conn) {
      try {
        auto peerAddr = conn->GetPeerAddress();
        auto localAddr = server.GetLocalAddress();
        fprintf(stdout,
                "%s -> %s is %s.\n",
                peerAddr.GetIPWithPort().c_str(),
                localAddr.GetIPWithPort().c_str(),
                conn->IsClosed() ? "down" : "up");

        conn->AsyncSend("Hello\n");
      } catch (const Exception &e) {
        fprintf(stdout, "%s\n", e.what());
      }
    });

    server.SetMessageCallback([this](AsyncTcpConnection *conn,
                                     AsyncTcpConnectionBuffer &buf,
                                     std::chrono::system_clock::time_point) {
      auto msg = buf.RetrieveAsString();
      fprintf(stdout, "received: %s\n", msg.c_str());

      if (msg >= "exit" && msg < "exiu") {
        conn->AsyncSend("Bye\n");
        conn->ShutdownWrite();
      }

      if (msg >= "quit" && msg < "quiu") {
        server.GetAcceptorLoop()->Quit();
      }

      conn->AsyncSend(msg);
    });
  }

  void Start() { server.Start(); }
};

int main(int argc, char **argv) {
  size_t numThread = std::thread::hardware_concurrency();
  if (argc > 1) {
    numThread = atoi(argv[1]);
    numThread = std::max<size_t>(numThread, 1);
  }

  try {
    Eventloop loop;
    InetAddress addr = InetAddress::Ipv4Any(2000);
    EchoServer server(loop, addr);
    fprintf(stdout, "Listening %s\n", addr.GetIPWithPort().c_str());
    server.Start();
    loop.Loop();
  } catch (const Exception &e) {
    fprintf(stdout, "%s\n", e.what());
  }
}
