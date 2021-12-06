#ifndef EVEIO_NET_ACCEPTOR_HPP
#define EVEIO_NET_ACCEPTOR_HPP

#include "eveio/Channel.hpp"
#include "eveio/net/TcpConnection.hpp"
#include "eveio/net/TcpSocket.hpp"

#include <functional>

namespace eveio {
namespace net {

using NewTcpConnectionCallback = std::function<void(TcpConnection &&)>;

class Acceptor {
  EventLoop *loop;
  TcpSocket accept_socket;
  Channel channel;
  NewTcpConnectionCallback callback;
  bool is_listening;

public:
  Acceptor(EventLoop &loop, TcpSocket &&socket, bool reuse_port) noexcept;

  Acceptor(const Acceptor &) = delete;
  Acceptor &operator=(const Acceptor &) = delete;

  Acceptor(Acceptor &&) = delete;
  Acceptor &operator=(Acceptor &&) = delete;

  ~Acceptor() noexcept;

  template <class Fn, class... Args>
  void SetNewConnectionCallback(Fn &&fn, Args &&...args) {
    callback = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
  }

  bool IsListening() const noexcept { return is_listening; }

  void Listen() noexcept;

private:
  void HandleRead(Time) noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_ACCEPTOR_HPP
