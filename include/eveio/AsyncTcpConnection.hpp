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

#ifndef EVEIO_ASYNC_TCP_CONNECTION_HPP
#define EVEIO_ASYNC_TCP_CONNECTION_HPP

#include "eveio/Channel.hpp"
#include "eveio/Socket.hpp"
#include "eveio/String.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>

namespace eveio {

class AsyncTcpConnectionBuffer {
  std::vector<char> storage;
  size_t head = 0, tail = 0;

public:
  AsyncTcpConnectionBuffer() noexcept = default;
  ~AsyncTcpConnectionBuffer() noexcept = default;

  AsyncTcpConnectionBuffer(const AsyncTcpConnectionBuffer &) noexcept = default;
  AsyncTcpConnectionBuffer &
  operator=(const AsyncTcpConnectionBuffer &) noexcept = default;

  AsyncTcpConnectionBuffer(AsyncTcpConnectionBuffer &&) noexcept = default;
  AsyncTcpConnectionBuffer &
  operator=(AsyncTcpConnectionBuffer &&) noexcept = default;

  template <class T>
  T *data() noexcept {
    return reinterpret_cast<T *>(storage.data() + head);
  }

  template <class T>
  const T *data() const noexcept {
    return reinterpret_cast<const T *>(storage.data() + head);
  }

  void clear() noexcept { head = tail = 0; }

  size_t size() const noexcept { return (tail - head); }
  size_t capacity() const noexcept { return (storage.capacity() - tail); }

  bool empty() const noexcept { return size() == 0; }

  /// This method is only used to move head pointer.
  void ReadOut(size_t byte) noexcept {
    head += byte;
    if (head >= tail) {
      clear();
    }
  }

  String RetrieveAsString() noexcept {
    String str(data<char>(), size());
    clear();
    return str;
  }

  void Append(const void *data, size_t size) noexcept;
  void Append(StringRef data) noexcept;
  void Append(const String &data) noexcept;

  char operator[](size_t i) const noexcept { return data<char>()[i]; }
  char &operator[](size_t i) noexcept { return data<char>()[i]; }
};

class AsyncTcpConnection;
class EventLoop;

using TcpMessageCallback =
    std::function<void(AsyncTcpConnection *,
                       AsyncTcpConnectionBuffer &,
                       std::chrono::system_clock::time_point)>;
using TcpWriteCompleteCallback = std::function<void(AsyncTcpConnection *)>;
using TcpConnectionCallback = std::function<void(AsyncTcpConnection *)>;

class AsyncTcpConnection
    : public std::enable_shared_from_this<AsyncTcpConnection> {
  std::shared_ptr<AsyncTcpConnection> guardThis;
  EventLoop *const loop;

  TcpConnection connection;
  Channel channel;

  AsyncTcpConnectionBuffer readBuffer;
  AsyncTcpConnectionBuffer writeBuffer;

  TcpMessageCallback messageCallback;
  TcpWriteCompleteCallback writeCompleteCallback;

  std::atomic_bool isQuit;

public:
  AsyncTcpConnection(EventLoop &ownerLoop, TcpConnection &&conn);
  ~AsyncTcpConnection() noexcept;

  AsyncTcpConnection(const AsyncTcpConnection &) noexcept = delete;
  AsyncTcpConnection &operator=(const AsyncTcpConnection &) noexcept = delete;

  AsyncTcpConnection(AsyncTcpConnection &&) noexcept = delete;
  AsyncTcpConnection &operator=(AsyncTcpConnection &&) noexcept = delete;

  /// For internal usage.
  void Initialize();

  void SetMessageCallback(TcpMessageCallback cb) noexcept {
    messageCallback = std::move(cb);
  }

  void SetWriteCompleteCallback(TcpWriteCompleteCallback cb) noexcept {
    writeCompleteCallback = std::move(cb);
  }

  EventLoop *GetLoop() const noexcept { return loop; }

  /// This method will require system socket API.
  /// Might be inefficient.
  ///
  /// Throw SystemErrorException if any error occurs.
  InetAddress GetPeerAddress() const { return connection.GetPeerAddress(); }

  bool ShutdownWrite() const noexcept { return connection.ShutdownWrite(); }
  bool IsClosed() const noexcept { return connection.IsClosed(); }

  bool SetNoDelay(bool on) const noexcept { return connection.SetNoDelay(on); }
  bool SetKeepAlive(bool on) const noexcept {
    return connection.SetKeepAlive(on);
  }

  void AsyncSend(const void *data, size_t size) noexcept;
  void AsyncSend(StringRef data) noexcept;
  void AsyncSend(const String &data) noexcept;
  void AsyncSend(const char *str) noexcept;

  void WouldDestroy() noexcept;

private:
  void Destroy() noexcept;
  void HandleRead(std::chrono::system_clock::time_point time) noexcept;
  void SendInLoop() noexcept;
};

} // namespace eveio

#endif // EVEIO_ASYNC_TCP_CONNECTION_HPP
