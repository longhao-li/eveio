#include "eveio/EventLoopThread.hpp"

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

eveio::EventLoopThread::EventLoopThread() noexcept
    : loop(nullptr), loop_thread(), mutex(), cond(), init_callback() {}

eveio::EventLoopThread::~EventLoopThread() noexcept {
  if (loop != nullptr) {
    loop->Quit();
    loop_thread.join();
  }
}

EventLoop *eveio::EventLoopThread::StartLoop() noexcept {
  loop_thread = std::thread(std::bind(&EventLoopThread::Task, this));
  std::unique_lock<std::mutex> lock(mutex);
  cond.wait(lock, [this]() -> bool { return this->loop != nullptr; });
  return loop;
}

void eveio::EventLoopThread::Task() noexcept {
  EventLoop task_loop;
  if (init_callback)
    init_callback(&task_loop);

  {
    std::lock_guard<std::mutex> lock(mutex);
    this->loop = &task_loop;
    cond.notify_one();
  }

  task_loop.Loop();

  std::lock_guard<std::mutex> lock(mutex);
  this->loop = nullptr;
}
