#ifndef EVEIO_NET_TCPSERVER_HPP
#define EVEIO_NET_TCPSERVER_HPP

#include "eveio/Channel.hpp"
#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/SmartPtr.hpp"
#include "eveio/net/Acceptor.hpp"
#include "eveio/net/AsyncTcpConnection.hpp"
#include "eveio/net/InetAddr.hpp"

#include <atomic>
#include <functional>

namespace eveio {
namespace net {

class TcpServer {
  EventLoop *const loop;
  EventLoopThreadPool *const io_context;
  std::atomic_bool is_started;
  bool reuse_port;

  UniquePtr<Acceptor> acceptor;

  TcpConnectionCallback connection_callback;
  TcpMessageCallback message_callback;
  TcpWriteCompleteCallback write_complete_callback;
  TcpCloseCallback close_callback;

  InetAddr local_addr;

public:
  TcpServer(EventLoop &loop,
            EventLoopThreadPool &io_context,
            const InetAddr &listen_addr,
            bool reuse_port) noexcept;

  TcpServer(const TcpServer &) = delete;
  TcpServer &operator=(const TcpServer &) = delete;

  TcpServer(TcpServer &&) = delete;
  TcpServer &operator=(TcpServer &&) = delete;

  ~TcpServer() noexcept = default;

  const InetAddr &LocalAddr() const noexcept { return acceptor->LocalAddr(); }
  EventLoop *GetLoop() const noexcept { return loop; }

  template <
      class Fn,
      class = std::enable_if_t<
          std::is_constructible<TcpConnectionCallback, std::decay_t<Fn>>::value,
          int>>
  void SetConnectionCallback(Fn &&fn) {
    connection_callback = TcpConnectionCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetConnectionCallback(Fn &&fn, Args &&...args) {
    connection_callback = std::bind(std::forward<Fn>(fn),
                                    std::forward<Args>(args)...,
                                    std::placeholders::_1);
  }

  template <
      class Fn,
      class = std::enable_if_t<
          std::is_constructible<TcpMessageCallback, std::decay_t<Fn>>::value,
          int>>
  void SetMessageCallback(Fn &&fn) {
    message_callback = TcpMessageCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetMessageCallback(Fn &&fn, Args &&...args) {
    message_callback = std::bind(std::forward<Fn>(fn),
                                 std::forward<Args>(args)...,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3);
  }

  template <
      class Fn,
      class = std::enable_if_t<std::is_constructible<TcpWriteCompleteCallback,
                                                     std::decay_t<Fn>>::value,
                               int>>
  void SetWriteCompleteCallback(Fn &&fn) {
    write_complete_callback = TcpWriteCompleteCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetWriteCompleteCallback(Fn &&fn, Args &&...args) {
    write_complete_callback = std::bind(std::forward<Fn>(fn),
                                        std::forward<Args>(args)...,
                                        std::placeholders::_1);
  }

  template <
      class Fn,
      class = std::enable_if_t<
          std::is_constructible<TcpCloseCallback, std::decay_t<Fn>>::value,
          int>>
  void SetCloseCallback(Fn &&fn) {
    close_callback = TcpCloseCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetCloseCallback(Fn &&fn, Args &&...args) {
    close_callback = std::bind(std::forward<Fn>(fn),
                               std::forward<Args>(args)...,
                               std::placeholders::_1);
  }

  void Start() noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_TCPSERVER_HPP
