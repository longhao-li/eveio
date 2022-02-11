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

#ifndef EVEIO_CHANNEL_HPP
#define EVEIO_CHANNEL_HPP

#include "eveio/Config.hpp"
#include "eveio/Event.hpp"

#include <cassert>
#include <chrono>
#include <functional>
#include <memory>

namespace eveio {

class EventLoop;

class Channel {
public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback =
      std::function<void(std::chrono::system_clock::time_point)>;

private:
  EventLoop *const loop;
  const handle_t handle;

  bool isTied;
  bool isHandlingEvent;
  bool isAddedToLoop;
  std::weak_ptr<void> tiedObject;

  Event eventsListing;
  Event eventsToHandle;

  int32_t pollerState;

  ReadEventCallback readCallback;
  EventCallback writeCallback;
  EventCallback errorCallback;
  EventCallback closeCallback;

public:
  Channel(EventLoop &loop, handle_t fd) noexcept;
  ~Channel() noexcept;

  Channel(const Channel &) = delete;
  Channel &operator=(const Channel &) = delete;

  Channel(Channel &&) = delete;
  Channel &operator=(Channel &&) = delete;

  template <class Fn>
  void SetReadCallback(Fn &&fn) noexcept {
    readCallback = ReadEventCallback(std::forward<Fn>(fn));
  }

  template <class Fn>
  void SetWriteCallback(Fn &&fn) noexcept {
    writeCallback = EventCallback(std::forward<Fn>(fn));
  }

  template <class Fn>
  void SetErrorCallback(Fn &&fn) noexcept {
    errorCallback = EventCallback(std::forward<Fn>(fn));
  }

  template <class Fn>
  void SetCloseCallback(Fn &&fn) noexcept {
    closeCallback = EventCallback(std::forward<Fn>(fn));
  }

  handle_t GetHandle() const noexcept { return handle; }
  Event EventsListening() const noexcept { return eventsListing; }

  /// If this channel relies on object, tie it to ensure callbacks runs
  /// currectly.
  void Tie(std::weak_ptr<void> p) noexcept {
    isTied = true;
    tiedObject = p;
  }

  /// For internal usage.
  /// Used by poller.
  void SetEventsToHandle(Event e) noexcept { eventsToHandle = e; }

  /// For internal usage.
  /// Used by poller.
  void AddEventsToHandle(Event e) noexcept { eventsToHandle |= e; }

  /// For internal usage.
  /// Used by poller.
  int32_t GetPollerState() const noexcept { return pollerState; }

  /// For internal usage.
  /// Used by poller.
  void SetPollerState(int32_t state) noexcept { pollerState = state; }

  bool IsNoneEvent() const noexcept { return EventsListening() == EVENT_NONE; }
  bool IsReading() const noexcept { return (EventsListening() & EVENT_READ); }
  bool IsWriting() const noexcept { return (EventsListening() & EVENT_WRITE); }

  /// For internal usage.
  /// Update state in poller.
  /// DO NOT call this method manually.
  void Update();

  /// For internal usage.
  /// Unregist from poller.
  /// DO NOT call this method manually.
  void Unregist();

  void EnableReading() noexcept {
    eventsListing |= EVENT_READ;
    Update();
  }

  void DisableReading() noexcept {
    eventsListing &= ~EVENT_READ;
    Update();
  }

  void EnableWriting() noexcept {
    eventsListing |= EVENT_WRITE;
    Update();
  }

  void DisableWriting() noexcept {
    eventsListing &= ~EVENT_WRITE;
    Update();
  }

  void DisableAll() noexcept {
    eventsListing = EVENT_NONE;
    Update();
  }

  EventLoop *GetOwnerLoop() const noexcept { return loop; }

  /// For internal usage.
  /// Handle all pending events.
  /// Called by Eventloop.
  /// DO NOT call tbis method manually.
  void HandleEvent(std::chrono::system_clock::time_point recvTime);
};

}; // namespace eveio

#endif // EVEIO_CHANNEL_HPP
