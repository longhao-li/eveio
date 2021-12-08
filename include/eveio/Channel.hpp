#ifndef EVEIO_CHANNEL_HPP
#define EVEIO_CHANNEL_HPP

#include "eveio/Event.hpp"
#include "eveio/Handle.hpp"
#include "eveio/SmartPtr.hpp"
#include "eveio/Time.hpp"

#include <functional>
#include <memory>

/// Muduo - A reactor-based C++ network library for Linux
/// Copyright (c) 2010, Shuo Chen.  All rights reserved.
/// http://code.google.com/p/muduo/
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions
/// are met:
///
///   * Redistributions of source code must retain the above copyright
/// notice, this list of conditions and the following disclaimer.
///   * Redistributions in binary form must reproduce the above copyright
/// notice, this list of conditions and the following disclaimer in the
/// documentation and/or other materials provided with the distribution.
///   * Neither the name of Shuo Chen nor the names of other contributors
/// may be used to endorse or promote products derived from this software
/// without specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
/// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
/// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
/// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
/// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
/// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
/// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
/// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
/// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

namespace eveio {

class EventLoop;

class Channel {
public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(eveio::Time)> ReadEventCallback;

private:
  EventLoop *const loop;
  const Handle handle;

  WeakPtr<void> tied_object;
  bool is_tied;
  bool is_handling_event;
  bool is_added_to_loop;

  Events events_listening;
  Events events_to_handle;

  int32_t poll_state;

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
    read_callback = std::bind(std::forward<Fn>(fn),
                              std::forward<Args>(args)...,
                              std::placeholders::_1);
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

  void Tie(WeakPtr<void> p) noexcept {
    is_tied = true;
    tied_object = p;
  }

  void SetEventsToHandle(Events e) noexcept { events_to_handle = e; }

  void AddEventsToHandle(Events e) noexcept { events_to_handle |= e; }

  uint32_t GetPollState() const noexcept { return poll_state; }

  void SetPollState(uint32_t state) noexcept { poll_state = state; }

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
