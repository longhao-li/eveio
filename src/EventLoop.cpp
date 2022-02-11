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

#include "eveio/EventLoop.hpp"
#include "eveio/Channel.hpp"
#include "eveio/Exception.hpp"
#include "eveio/Poller.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <exception>

using namespace eveio;

static thread_local eveio::EventLoop *LoopInCurrentThread = nullptr;

eveio::EventLoop::EventLoop()
    : poller(new Poller),
      isLooping(false),
      isQuit(false),
      isHandlingEvent(false),
      wakeupHandle(),
      wakeupChannel(),
      threadID(GetThreadID()),
      activeChannels(),
      pendingFunc(),
      pendingFuncMutex() {
  if (LoopInCurrentThread != nullptr) {
    throw RuntimeErrorException(
        __FILENAME__,
        __LINE__,
        __func__,
        String("Another eventloop ") +
            std::to_string(reinterpret_cast<uint64_t>(LoopInCurrentThread)) +
            " already exists in current thread " + std::to_string(threadID) +
            ".");
  } else {
    LoopInCurrentThread = this;
  }

  wakeupChannel.reset(new Channel(*this, wakeupHandle.GetListenHandle()));
  wakeupChannel->SetReadCallback([this](std::chrono::system_clock::time_point) {
    this->wakeupHandle.Respond();
  });
  wakeupChannel->EnableReading();
}

eveio::EventLoop::~EventLoop() noexcept {
  wakeupChannel->DisableAll();
  wakeupChannel->Unregist();
  assert(LoopInCurrentThread == this);
  LoopInCurrentThread = nullptr;
}

void eveio::EventLoop::Loop() noexcept {
  assert(!isLooping.load(std::memory_order_relaxed));
  if (isLooping.exchange(true, std::memory_order_relaxed)) {
    return;
  }

  while (!isQuit.load(std::memory_order_relaxed)) {
    activeChannels.clear();

    try {
      auto pollReturnTime =
          poller->Poll(std::chrono::milliseconds(10000), activeChannels);
      {
        isHandlingEvent.store(true, std::memory_order_relaxed);
        for (const auto &channel : activeChannels) {
          channel->HandleEvent(pollReturnTime);
        }
        isHandlingEvent.store(false, std::memory_order_relaxed);
      }
    } catch (const SystemErrorException &e) {
      // TODO: Handle error
    }

    DoPendingFunc();
  }

  isLooping.store(false, std::memory_order_relaxed);
}

void eveio::EventLoop::Quit() noexcept {
  isQuit.store(true, std::memory_order_relaxed);
  if (!IsInLoopThread()) {
    WakeUp();
  }
}

void eveio::EventLoop::DoPendingFunc() noexcept {
  std::vector<std::function<void()>> pendingFuncCopy;
  {
    std::lock_guard<std::mutex> lock(pendingFuncMutex);
    pendingFuncCopy.swap(pendingFunc);
  }

  for (const auto &func : pendingFuncCopy) {
    func();
  }
}

EventLoop *eveio::EventLoop::GetCurrentThreadLoop() noexcept {
  return LoopInCurrentThread;
}

void eveio::EventLoop::UpdateChannel(Channel *channel) const {
  assert(channel->GetOwnerLoop() == this);
  poller->UpdateChannel(channel);
}

void eveio::EventLoop::UnregistChannel(Channel *channel) const {
  assert(channel->GetOwnerLoop() == this);
  poller->UnregistChannel(channel);
}
