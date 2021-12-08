#include "eveio/Channel.hpp"
#include "eveio/Event.hpp"
#include "eveio/EventLoop.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

using namespace eveio;

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

eveio::Channel::Channel(EventLoop &loop, Handle hd) noexcept
    : loop(&loop),
      handle(hd),
      tied_object(),
      is_tied(false),
      is_handling_event(false),
      is_added_to_loop(false),
      events_listening(event::NoneEvent),
      events_to_handle(event::NoneEvent),
      poll_state(Poller::ChannelInitPollState),
      read_callback(),
      write_callback(),
      close_callback(),
      error_callback() {}

eveio::Channel::~Channel() noexcept {
  assert(!is_handling_event);
  assert(!is_added_to_loop);
}

bool eveio::Channel::IsRegisted() const noexcept {
  return loop->poller.HasChannel(*this);
}

void eveio::Channel::Update() noexcept {
  loop->poller.UpdateChannel(*this);
  is_added_to_loop = true;
}

void eveio::Channel::Unregist() noexcept {
  loop->poller.RemoveChannel(*this);
  is_added_to_loop = false;
}

void eveio::Channel::HandleEvent(Time recv_time) {
  if (events_to_handle == event::NoneEvent)
    return;

  SharedPtr<void> guard;
  if (is_tied) {
    guard = tied_object.lock();
    if (!guard) {
      SPDLOG_TRACE("tied object released.");
      return;
    }
  }

  is_handling_event = true;
  if ((events_to_handle & event::CloseEvent) &&
      !(events_to_handle & event::ReadEvent)) {
    if (close_callback)
      close_callback();
  }

  if (events_to_handle & event::ErrorEvent) {
    if (error_callback)
      error_callback();
  }

  if (events_to_handle & event::ReadEvent) {
    if (read_callback)
      read_callback(recv_time);
  }

  if (events_to_handle & event::WriteEvent) {
    if (write_callback)
      write_callback();
  }

  events_to_handle = event::NoneEvent;
  is_handling_event = false;
}
