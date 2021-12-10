#include "eveio/EventLoop.hpp"
#include "eveio/WakeupHandle.hpp"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdlib>

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

static thread_local eveio::EventLoop *LoopInCurrentThread = nullptr;

eveio::EventLoop::EventLoop() noexcept
    : poller(),
      is_looping(false),
      is_quit(false),
      is_handling_event(false),
      wakeup_handle(),
      wakeup_channel(),
      t_id(std::this_thread::get_id()),
      active_channels(),
      pending_func() {
  if (LoopInCurrentThread != nullptr) {
    SPDLOG_CRITICAL("Another eventloop {} already exists in current thread {}.",
                    static_cast<void *>(LoopInCurrentThread),
                    t_id);
    std::abort();
  } else {
    LoopInCurrentThread = this;
  }

  wakeup_channel = MakeUnique<Channel>(*this, wakeup_handle.ListenHandle());
  wakeup_channel->SetReadCallback(
      [this](Time) { this->wakeup_handle.Respond(); });
  wakeup_channel->EnableReading();
}

eveio::EventLoop::~EventLoop() noexcept {
  wakeup_channel->DisableAll();
  wakeup_channel->Unregist();
  WakeupHandle::Close(wakeup_handle);
  LoopInCurrentThread = nullptr;
}

void eveio::EventLoop::Loop() noexcept {
  assert(!is_looping.load(std::memory_order_relaxed));
  is_looping.store(true, std::memory_order_relaxed);
  is_quit.store(false, std::memory_order_relaxed);

  while (!is_quit.load(std::memory_order_relaxed)) {
    active_channels.clear();
    Time poll_return_time =
        poller.Poll(Time::Milliseconds(10000), active_channels);

    { // handle events
      is_handling_event = true;
      for (Channel *const chan : active_channels)
        chan->HandleEvent(poll_return_time);
      is_handling_event = false;
    }

    DoPendingFunc();
  }

  is_looping.store(false, std::memory_order_relaxed);
}

void eveio::EventLoop::Quit() noexcept {
  is_quit.store(true, std::memory_order_relaxed);
  if (!IsInLoopThread())
    WakeUp();
}

void eveio::EventLoop::DoPendingFunc() noexcept {
  std::function<void()> fn;
  while (pending_func.try_dequeue(fn))
    fn();
}

EventLoop *eveio::EventLoop::CurrentThreadLoop() noexcept {
  return LoopInCurrentThread;
}
