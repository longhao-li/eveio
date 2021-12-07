#ifndef EVEIO_NET_ACCEPTOR_HPP
#define EVEIO_NET_ACCEPTOR_HPP

#include "eveio/Channel.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/TcpConnection.hpp"
#include "eveio/net/TcpSocket.hpp"

#include <functional>

namespace eveio {
namespace net {

using NewTcpConnectionCallback = std::function<void(TcpConnection &&)>;

class Acceptor : public std::enable_shared_from_this<Acceptor> {
  EventLoop *loop;
  volatile bool is_listening;
  TcpSocket accept_socket;
  Channel channel;
  NewTcpConnectionCallback callback;

public:
  Acceptor(EventLoop &loop, TcpSocket &&socket, bool reuse_port) noexcept;

  Acceptor(const Acceptor &) = delete;
  Acceptor &operator=(const Acceptor &) = delete;

  Acceptor(Acceptor &&) = delete;
  Acceptor &operator=(Acceptor &&) = delete;

  ~Acceptor() noexcept;

  template <
      class Fn,
      class = std::enable_if_t<std::is_constructible<NewTcpConnectionCallback,
                                                     std::decay_t<Fn>>::value,
                               int>>
  void SetNewConnectionCallback(Fn &&fn) {
    callback = NewTcpConnectionCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetNewConnectionCallback(Fn &&fn, Args &&...args) {
    callback = std::bind(std::forward<Fn>(fn),
                         std::forward<Args>(args)...,
                         std::placeholders::_1);
  }

  bool IsListening() const noexcept { return is_listening; }

  const InetAddr &LocalAddr() const noexcept {
    return accept_socket.LocalAddr();
  }

  void Listen() noexcept;

  void Quit() noexcept;

private:
  void HandleRead(Time) noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_ACCEPTOR_HPP
