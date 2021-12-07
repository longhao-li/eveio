#include "eveio/EventLoop.hpp"
#include "eveio/WakeupHandle.hpp"

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdlib>

using namespace eveio;

static thread_local eveio::EventLoop *LoopInCurrentThread = nullptr;

eveio::EventLoop::EventLoop() noexcept
    : is_looping(false),
      is_quit(false),
      is_handling_event(false),
      wakeup_handle(),
      wakeup_channel(),
      poller(),
      active_channels(),
      t_id(std::this_thread::get_id()),
      mutex(),
      pending_func() {
  if (LoopInCurrentThread != nullptr) {
    SPDLOG_CRITICAL("Another eventloop {} already exists in current thread {}.",
                    static_cast<void *>(LoopInCurrentThread),
                    t_id);
    std::abort();
  } else {
    LoopInCurrentThread = this;
  }

  wakeup_channel =
      std::make_unique<Channel>(*this, wakeup_handle.ListenHandle());
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
  Vector<std::function<void()>> funcs;
  {
    std::lock_guard<std::mutex> lock(mutex);
    pending_func.swap(funcs);
  }

  for (auto &&fn : funcs)
    fn();
}

EventLoop *eveio::EventLoop::CurrentThreadLoop() noexcept {
  return LoopInCurrentThread;
}
