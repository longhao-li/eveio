#ifndef EVEIO_CHANNEL_HPP
#define EVEIO_CHANNEL_HPP

#include "eveio/Event.hpp"
#include "eveio/Handle.hpp"
#include "eveio/Time.hpp"

#include <functional>
#include <memory>

namespace eveio {

class EventLoop;

class Channel {
public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(eveio::Time)> ReadEventCallback;

private:
  EventLoop *const loop;
  const Handle handle;

  std::weak_ptr<void> tied_object;
  bool is_tied;

  Events events_listening;
  Events events_to_handle;

  bool is_handling_event;
  bool is_added_to_loop;

  ReadEventCallback read_callback;
  EventCallback write_callback;
  EventCallback close_callback;
  EventCallback error_callback;

public:
  Channel(EventLoop &loop, Handle hd) noexcept;

  Channel(const Channel &) = delete;
  Channel &operator=(const Channel &) = delete;

  Channel(Channel &&) = delete;
  Channel &operator=(Channel &&) = delete;

  ~Channel() noexcept;

  template <class Fn, class... Args>
  void SetReadCallback(Fn &&fn, Args &&...args) {
    read_callback =
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
  }

  template <class Fn, class... Args>
  void SetWriteCallback(Fn &&fn, Args &&...args) {
    write_callback =
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
  }

  template <class Fn, class... Args>
  void SetCloseCallback(Fn &&fn, Args &&...args) {
    close_callback =
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
  }

  template <class Fn, class... Args>
  void SetErrorCallback(Fn &&fn, Args &&...args) {
    error_callback =
        std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
  }

  Handle GetHandle() const noexcept { return handle; }

  Events ListeningEvents() const noexcept { return events_listening; }

  void SetEventsToHandle(Events e) noexcept { events_to_handle = e; }

  void AddEventsToHandle(Events e) noexcept { events_to_handle |= e; }

  bool IsNoneEvent() const noexcept {
    return events_listening == event::NoneEvent;
  }

  bool IsReading() const noexcept {
    return (events_listening & event::ReadEvent);
  }

  bool IsWriting() const noexcept {
    return (events_listening & event::WriteEvent);
  }

  void EnableReading() noexcept {
    events_listening |= event::ReadEvent;
    Update();
  }

  void EnableWriting() noexcept {
    events_listening |= event::WriteEvent;
    Update();
  }

  void DisableReading() noexcept {
    events_listening &= (~event::ReadEvent);
    Update();
  }

  void DisableWriting() noexcept {
    events_listening &= (~event::WriteEvent);
    Update();
  }

  void DisableAll() noexcept {
    events_listening = event::NoneEvent;
    Update();
  }

  EventLoop &OwnerLoop() const noexcept { return *loop; }

  bool IsRegisted() const noexcept;

  // 内部使用，不要手动调用Update。
  void Update() noexcept;

  void Unregist() noexcept;

  // 本函数本体不会抛出异常。考虑到callback可能抛出异常，没有加上noexcept。但是强烈建议不要抛出异常，因为无法catch。
  void HandleEvent(Time recv_time);
};

} // namespace eveio

#endif // EVEIO_CHANNEL_HPP
