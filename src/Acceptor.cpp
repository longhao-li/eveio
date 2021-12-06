#include "eveio/net/Acceptor.hpp"
#include "eveio/Channel.hpp"
#include "eveio/EventLoop.hpp"
#include "eveio/net/TcpConnection.hpp"
#include "eveio/net/TcpSocket.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdlib>

using eveio::EventLoop;
using eveio::net::Acceptor;
using eveio::net::TcpSocket;

eveio::net::Acceptor::Acceptor(EventLoop &loop,
                               TcpSocket &&socket,
                               bool reuse_port) noexcept
    : loop(&loop),
      is_listening(false),
      accept_socket(std::move(socket)),
      channel(loop, accept_socket.native_socket()),
      callback() {
  if (!accept_socket.SetReuseAddr(true)) {
    SPDLOG_CRITICAL("socket {} failed to set reuse addr. abort.",
                    accept_socket.native_socket());
    std::abort();
  }

  if (reuse_port && !accept_socket.SetReusePort(true)) {
    SPDLOG_CRITICAL("socket {} failed to set reuse port. abort.",
                    accept_socket.native_socket());
    std::abort();
  }

  channel.SetReadCallback(&Acceptor::HandleRead, this);
}

eveio::net::Acceptor::~Acceptor() noexcept { channel.Unregist(); }

void eveio::net::Acceptor::Listen() noexcept {
  assert(loop->IsInLoopThread());
  is_listening = true;
  if (!accept_socket.Listen(SOMAXCONN)) {
    SPDLOG_CRITICAL("socket {} failed to listen. abort.",
                    accept_socket.native_socket());
    std::abort();
  }
  channel.EnableReading();
}

void eveio::net::Acceptor::HandleRead(Time) noexcept {
  assert(loop->IsInLoopThread());
  auto conn_res = accept_socket.Accept();
  if (!conn_res.IsValid()) {
    SPDLOG_ERROR("acceptor {} failed to accept: {}.",
                 accept_socket.native_socket(),
                 conn_res.GetError());
  } else if (callback) {
    callback(std::move(conn_res.GetValue()));
  }
}
