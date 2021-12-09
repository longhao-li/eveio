#ifndef EVEIO_NET_UDPSERVER_HPP
#define EVEIO_NET_UDPSERVER_HPP

#include "eveio/Channel.hpp"
#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/net/Buffer.hpp"
#include "eveio/net/InetAddr.hpp"
#include "eveio/net/UdpSocket.hpp"

#include <atomic>

namespace eveio {
namespace net {

class UdpServer;

typedef std::function<void(
    UdpServer *, const InetAddr &, FixedBuffer &, Time, int)>
    UdpMessageCallback;
typedef std::function<void(UdpServer *, const InetAddr &, int)>
    UdpWriteCompleteCallback;

class UdpServer {
  EventLoop *const loop;

  std::atomic_bool is_started;

  UniquePtr<UdpSocket> sock;
  SharedPtr<Channel> channel;

  UdpMessageCallback message_callback;
  UdpWriteCompleteCallback write_complete_callback;

  std::mutex mutex;
  Vector<std::pair<FixedBuffer, InetAddr>> write_buffer;

public:
  UdpServer(EventLoop &loop,
            const InetAddr &listen_addr,
            bool reuse_port) noexcept;

  UdpServer(const UdpServer &) = delete;
  UdpServer &operator=(const UdpServer &) = delete;

  UdpServer(UdpServer &&) = delete;
  UdpServer &operator=(UdpServer &&) = delete;

  ~UdpServer() noexcept;

  EventLoop *GetLoop() const noexcept { return loop; }

  const InetAddr &ListeningAddr() const noexcept { return sock->LocalAddr(); }

  template <
      class Fn,
      class = std::enable_if_t<
          std::is_constructible<UdpMessageCallback, std::decay_t<Fn>>::value,
          int>>
  void SetMessageCallback(Fn &&fn) {
    message_callback = UdpMessageCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetMessageCallback(Fn &&fn, Args &&...args) {
    message_callback = std::bind(std::forward<Fn>(fn),
                                 std::forward<Args>(args)...,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3,
                                 std::placeholders::_4,
                                 std::placeholders::_5);
  }

  template <
      class Fn,
      class = std::enable_if_t<std::is_constructible<UdpWriteCompleteCallback,
                                                     std::decay_t<Fn>>::value,
                               int>>
  void SetWriteCompleteCallback(Fn &&fn) {
    write_complete_callback = UdpWriteCompleteCallback(std::forward<Fn>(fn));
  }

  template <class Fn, class... Args>
  void SetWriteCompleteCallback(Fn &&fn, Args &&...args) {
    write_complete_callback = std::bind(std::forward<Fn>(fn),
                                        std::forward<Args>(args)...,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3);
  }

  bool AsyncSend(StringRef data, const InetAddr &peer) noexcept;

  bool AsyncSend(const void *data, size_t sz, const InetAddr &peer) noexcept;

  void Start() noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_UDPSERVER_HPP
