#include "eveio/EventLoopThreadPool.h"
#include "eveio/EventLoop.h"

using namespace eveio;

eveio::EventLoopThreadPool::EventLoopThreadPool(size_t num_thread) noexcept
    : m_is_started(false),
      m_num_threads(num_thread),
      m_next_loop(0),
      m_workers(),
      m_loops() {}

EventLoop *eveio::EventLoopThreadPool::GetNextLoop() noexcept {
    if (m_is_started.load(std::memory_order_relaxed)) {
        return m_loops[m_next_loop.fetch_add(1, std::memory_order_relaxed) %
                       m_loops.size()];
    }
    return nullptr;
}

void eveio::EventLoopThreadPool::Start() noexcept {
    if (m_is_started.exchange(true, std::memory_order_relaxed))
        return;

    size_t num_threads =
        std::max(m_num_threads.load(std::memory_order_relaxed), size_t(1));

    m_workers.reserve(num_threads);
    m_loops.reserve(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        m_workers.emplace_back(new EventLoopThread);
        m_loops.emplace_back(m_workers.back()->StartLoop());
    }
}
