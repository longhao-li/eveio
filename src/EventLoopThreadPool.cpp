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

#include "eveio/EventloopThreadPool.hpp"
#include "eveio/Eventloop.hpp"
#include "eveio/EventloopThread.hpp"

#include <atomic>

using namespace eveio;

eveio::EventloopThreadPool::EventloopThreadPool(size_t threadNum) noexcept
    : isStarted(false), numThread(threadNum), nextLoop(0), workers(), loops() {}

Eventloop *eveio::EventloopThreadPool::GetNextLoop() noexcept {
  if (isStarted.load(std::memory_order_relaxed)) {
    return loops[nextLoop.fetch_add(1, std::memory_order_relaxed) %
                 loops.size()];
  }
  return nullptr;
}

void eveio::EventloopThreadPool::Start() noexcept {
  if (isStarted.exchange(true, std::memory_order_relaxed)) {
    return;
  }

  assert(numThread > 0);
  for (size_t i = 0; i < numThread; ++i) {
    workers.emplace_back(new EventloopThread);
    loops.emplace_back(workers.back()->StartLoop());
  }
}
