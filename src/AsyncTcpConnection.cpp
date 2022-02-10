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

#include "eveio/AsyncTcpConnection.hpp"
#include "eveio/Eventloop.hpp"
#include "eveio/Exception.hpp"

#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <memory>

using namespace eveio;

void eveio::AsyncTcpConnectionBuffer::Append(const void *ptr,
                                             size_t size) noexcept {
  storage.reserve(tail + size);
  memcpy(storage.data() + tail, ptr, size);
  tail += size;
}

eveio::AsyncTcpConnection::AsyncTcpConnection(Eventloop &ownerLoop,
                                              TcpConnection &&conn)
    : std::enable_shared_from_this<AsyncTcpConnection>(),
      guardThis(),
      loop(&ownerLoop),
      connection(std::move(conn)),
      channel(ownerLoop, reinterpret_cast<handle_t>(connection.GetSocket())),
      readBuffer(),
      writeBuffer(),
      messageCallback(),
      writeCompleteCallback(),
      isQuit(false) {}

void eveio::AsyncTcpConnection::Initialize() {
  guardThis = shared_from_this();

  if (!connection.SetNonblock(true)) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               "Failed to set connection " +
                                   std::to_string(connection.GetSocket()) +
                                   " nonblock: " + std::strerror(errno));
  }

  if (!connection.SetKeepAlive(true)) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               "Failed to set connection " +
                                   std::to_string(connection.GetSocket()) +
                                   " keepalive: " + std::strerror(errno));
  }

  channel.Tie(shared_from_this());
  channel.SetReadCallback([this](std::chrono::system_clock::time_point time) {
    this->HandleRead(time);
  });
  channel.SetWriteCallback([this]() { this->SendInLoop(); });
  channel.SetCloseCallback([this]() { this->WouldDestroy(); });
  loop->RunInLoop([this]() { this->channel.EnableReading(); });
}

eveio::AsyncTcpConnection::~AsyncTcpConnection() noexcept {
  assert(loop->IsInLoopThread());
  assert(channel.IsNoneEvent());
}

void eveio::AsyncTcpConnection::AsyncSend(const void *data,
                                          size_t size) noexcept {
  if (isQuit.load(std::memory_order_relaxed)) {
    return;
  }

  if (loop->IsInLoopThread()) {
    writeBuffer.Append(data, size);
    SendInLoop();
  } else {
    String buf(static_cast<const char *>(data), size);
    loop->RunInLoop([this, buf]() {
      this->writeBuffer.Append(buf.data(), buf.size());
      this->channel.EnableWriting();
    });
  }
}

void eveio::AsyncTcpConnection::AsyncSend(StringRef data) noexcept {
  AsyncSend(data.data(), data.size());
}

void eveio::AsyncTcpConnection::AsyncSend(const String &data) noexcept {
  AsyncSend(data.data(), data.size());
}

void eveio::AsyncTcpConnection::AsyncSend(const char *str) noexcept {
  AsyncSend(StringRef(str));
}

void eveio::AsyncTcpConnection::WouldDestroy() noexcept {
  if (isQuit.exchange(true, std::memory_order_relaxed) == false) {
    auto guard = shared_from_this();
    loop->QueueInLoop([guard]() { guard->Destroy(); });
  }
}

void eveio::AsyncTcpConnection::Destroy() noexcept {
  assert(loop->IsLooping());
  assert(loop->IsInLoopThread());

  channel.DisableAll();
  channel.Unregist();

  guardThis = nullptr;
}

void eveio::AsyncTcpConnection::HandleRead(
    std::chrono::system_clock::time_point time) noexcept {
  assert(loop->IsLooping());
  assert(loop->IsInLoopThread());

  auto tryRead = [this, time]() {
    if (this->readBuffer.size() > 0) {
      if (this->messageCallback) {
        messageCallback(this, readBuffer, time);
      } else {
        readBuffer.clear();
      }
    }
  };

  char buffer[4096]{};
  int64_t byteRead = 0;
  while ((byteRead = connection.Receive(buffer, sizeof(buffer))) > 0) {
    readBuffer.Append(buffer, byteRead);
  }

  int savedErrno = errno;
  if (byteRead >= 0 || (byteRead < 0 && savedErrno == EWOULDBLOCK)) {
    tryRead();
  } else {
    if (savedErrno == ECONNRESET || savedErrno == EPIPE) {
      tryRead();
      WouldDestroy();
    } else {
      // TODO: Handle error
    }
  }
}

void eveio::AsyncTcpConnection::SendInLoop() noexcept {
  assert(loop->IsLooping());
  assert(loop->IsInLoopThread());

  if (writeBuffer.empty()) {
    return;
  }

  int64_t byteWritten =
      connection.Send(writeBuffer.data<char>(), writeBuffer.size());
  if (byteWritten >= 0) {
    writeBuffer.ReadOut(byteWritten);
    if (writeBuffer.empty()) {
      channel.DisableWriting();
      if (writeCompleteCallback) {
        writeCompleteCallback(this);
      }
    }
  } else {
    int savedErrno = errno;
    if (savedErrno == ECONNRESET || savedErrno == EPIPE) {
      // Connection closed
      WouldDestroy();
      return;
    } else if (savedErrno != EWOULDBLOCK) {
      // TODO: Handle error
      WouldDestroy();
      return;
    }
  }

  if (!writeBuffer.empty()) {
    channel.EnableWriting();
  }
}
