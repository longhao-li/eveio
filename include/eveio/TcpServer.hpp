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

#ifndef EVEIO_TCP_SERVER_HPP
#define EVEIO_TCP_SERVER_HPP

#include "eveio/AsyncTcpConnection.hpp"
#include "eveio/Channel.hpp"
#include "eveio/InetAddress.hpp"
#include "eveio/Socket.hpp"

#include <atomic>
#include <memory>

namespace eveio {

class EventLoop;

class Acceptor {
  EventLoop *const loop;
  bool isListening;
  TcpSocket acceptSocket;
  Channel channel;
  std::function<void(TcpConnection &&)> newConnectionCallback;

public:
  /// Throw SystemErrorException if failed to create socket or failed to config
  /// reuseport.
  Acceptor(EventLoop &ownerLoop, const InetAddress &listenAddr, bool reusePort);
  ~Acceptor() noexcept;

  Acceptor(const Acceptor &) = delete;
  Acceptor &operator=(const Acceptor &) = delete;

  Acceptor(Acceptor &&) = delete;
  Acceptor &operator=(Acceptor &&) = delete;

  void
  SetNewConnectionCallback(std::function<void(TcpConnection &&)> cb) noexcept {
    this->newConnectionCallback = std::move(cb);
  }

  /// NOT Thread safe.
  bool IsListening() const noexcept { return this->isListening; }

  /// Throw SystemErrorException if any error occurs.
  InetAddress GetLocalAddress() const { return acceptSocket.GetLocalAddress(); }

  /// Throw SystemErrorException if failed to listen.
  void Listen();
  void Quit() noexcept;
};

class EventLoopThreadPool;

class TcpServer {
  EventLoop *const acceptorLoop;
  std::shared_ptr<EventLoopThreadPool> ioContext;
  std::shared_ptr<Acceptor> acceptor;
  std::atomic_bool isStarted;

  TcpConnectionCallback connectionCallback;
  TcpMessageCallback messageCallback;
  TcpWriteCompleteCallback writeCompleteCallback;

public:
  TcpServer(EventLoop &loop, const InetAddress &listenAddr);
  TcpServer(EventLoop &loop,
            const InetAddress &listenAddr,
            std::shared_ptr<EventLoopThreadPool> threadPool);
  ~TcpServer() noexcept;

  TcpServer(const TcpServer &) = delete;
  TcpServer &operator=(const TcpServer &) = delete;

  TcpServer(TcpServer &&) = delete;
  TcpServer &operator=(TcpServer &&) = delete;

  /// Throw SystemErrorException if failed to get local addr.
  InetAddress GetLocalAddress() const { return acceptor->GetLocalAddress(); }

  std::shared_ptr<EventLoopThreadPool> GetIOContext() const noexcept {
    return ioContext;
  }

  EventLoop *GetAcceptorLoop() const noexcept { return acceptorLoop; }

  void SetConnectionCallback(TcpConnectionCallback cb) noexcept {
    connectionCallback = cb;
  }

  void SetMessageCallback(TcpMessageCallback cb) noexcept {
    messageCallback = cb;
  }

  void SetWriteCompleteCallback(TcpWriteCompleteCallback cb) noexcept {
    writeCompleteCallback = cb;
  }

  void Start();
};

} // namespace eveio

#endif // EVEIO_TCP_SERVER_HPP
