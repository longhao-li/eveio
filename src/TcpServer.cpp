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

#include "eveio/TcpServer.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/Exception.hpp"
#include "eveio/InetAddress.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>

using namespace eveio;

eveio::Acceptor::Acceptor(EventLoop &ownerLoop,
                          const InetAddress &listenAddr,
                          bool reusePort)
    : loop(&ownerLoop),
      isListening(false),
      acceptSocket(listenAddr),
      channel(ownerLoop, reinterpret_cast<handle_t>(acceptSocket.GetSocket())),
      newConnectionCallback() {
  if (!acceptSocket.SetReuseAddr(true)) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               "Failed to set reuse addr for " +
                                   listenAddr.GetIPWithPort() + ": " +
                                   std::strerror(errno));
  }

  acceptSocket.SetReusePort(reusePort);

  if (!acceptSocket.SetNonblock(true)) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               "Failed to set accept socket " +
                                   std::to_string(acceptSocket.GetSocket()) +
                                   " nonblock: " + std::strerror(errno));
  }

  channel.SetReadCallback([this](std::chrono::system_clock::time_point) {
    assert(loop->IsInLoopThread());
    auto conn = acceptSocket.Accept();
    if (conn.GetSocket() != INVALID_SOCKET && newConnectionCallback) {
      newConnectionCallback(std::move(conn));
    }
  });
}

eveio::Acceptor::~Acceptor() noexcept { assert(channel.IsNoneEvent()); }

void eveio::Acceptor::Listen() {
  assert(loop->IsInLoopThread());
  isListening = true;
  if (!acceptSocket.Listen(SOMAXCONN)) {
    throw SystemErrorException(
        __FILENAME__,
        __LINE__,
        __func__,
        "socket " + std::to_string(acceptSocket.GetSocket()) +
            " failed to listen: " + std::strerror(errno));
  }
  loop->RunInLoop([this]() { this->channel.EnableReading(); });
}

void eveio::Acceptor::Quit() noexcept {
  assert(loop->IsInLoopThread());
  channel.DisableAll();
  channel.Unregist();
  isListening = false;
}

eveio::TcpServer::TcpServer(EventLoop &loop, const InetAddress &listenAddr)
    : acceptorLoop(&loop),
      ioContext(),
      acceptor(std::make_shared<Acceptor>(loop, listenAddr, true)),
      isStarted(false),
      connectionCallback(),
      messageCallback(),
      writeCompleteCallback() {
  acceptor->SetNewConnectionCallback([this](TcpConnection &&conn) {
    try {
      auto asyncConn = std::make_shared<AsyncTcpConnection>(
          *(this->ioContext->GetNextLoop()), std::move(conn));
      asyncConn->Initialize();

      if (this->messageCallback) {
        asyncConn->SetMessageCallback(messageCallback);
      }

      if (this->writeCompleteCallback) {
        asyncConn->SetWriteCompleteCallback(writeCompleteCallback);
      }

      if (connectionCallback) {
        connectionCallback(asyncConn.get());
      }
    } catch (const Exception &e) {
      // TODO: Handle error
    }
  });
}

eveio::TcpServer::TcpServer(EventLoop &loop,
                            const InetAddress &listenAddr,
                            std::shared_ptr<EventLoopThreadPool> threadPool)
    : acceptorLoop(&loop),
      ioContext(std::move(threadPool)),
      acceptor(std::make_shared<Acceptor>(loop, listenAddr, true)),
      isStarted(false),
      connectionCallback(),
      messageCallback(),
      writeCompleteCallback() {
  acceptor->SetNewConnectionCallback([this](TcpConnection &&conn) {
    try {
      auto asyncConn = std::make_shared<AsyncTcpConnection>(
          *(this->ioContext->GetNextLoop()), std::move(conn));
      asyncConn->Initialize();

      if (this->messageCallback) {
        asyncConn->SetMessageCallback(messageCallback);
      }

      if (this->writeCompleteCallback) {
        asyncConn->SetWriteCompleteCallback(writeCompleteCallback);
      }

      if (connectionCallback) {
        connectionCallback(asyncConn.get());
      }
    } catch (const Exception &e) {
      // TODO: Handle error
    }
  });
}

eveio::TcpServer::~TcpServer() noexcept {
  {
    std::shared_ptr<Acceptor> guard(acceptor);
    acceptorLoop->RunInLoop([guard]() { guard->Quit(); });
  }
}

void eveio::TcpServer::Start() {
  if (isStarted.exchange(true, std::memory_order_relaxed)) {
    return;
  }

  ioContext->Start();
  acceptorLoop->RunInLoop([this]() { acceptor->Listen(); });
}
