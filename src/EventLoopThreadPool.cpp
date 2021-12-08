#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/EventLoopThread.hpp"

#include <spdlog/spdlog.h>

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

eveio::EventLoopThreadPool::EventLoopThreadPool(size_t thread_num) noexcept
    : is_started(false),
      num_thread(thread_num > 0 ? thread_num : 1),
      next_loop(0),
      workers(),
      loops() {}

EventLoop *eveio::EventLoopThreadPool::GetNextLoop() noexcept {
  if (is_started.load(std::memory_order_relaxed))
    return loops[next_loop.fetch_add(1, std::memory_order_relaxed) %
                 loops.size()];
  return nullptr;
}

Vector<EventLoop *> eveio::EventLoopThreadPool::GetAllLoops() const noexcept {
  return loops;
}

void eveio::EventLoopThreadPool::Start() noexcept {
  if (is_started.exchange(true, std::memory_order_relaxed) == false) {
    if (num_thread <= 0) {
      SPDLOG_CRITICAL("num_thread is 0.");
      std::abort();
    }
    for (size_t i = 0; i < num_thread; ++i) {
      workers.emplace_back(MakeUnique<EventLoopThread>());
      loops.emplace_back(workers.back()->StartLoop());
    }
  }
}
