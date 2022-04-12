#pragma once

#include "eveio/EventLoopThread.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <vector>

namespace eveio {

class EventLoopThreadPool {
public:
    using LoopList = std::vector<EventLoop *>;

public:
    explicit EventLoopThreadPool(
        size_t num_thread = std::thread::hardware_concurrency()) noexcept;
    ~EventLoopThreadPool() = default;

    EventLoopThreadPool(const EventLoopThreadPool &) = delete;
    EventLoopThreadPool &operator=(const EventLoopThreadPool &) = delete;

    EventLoopThreadPool(EventLoopThreadPool &&) = delete;
    EventLoopThreadPool &operator=(EventLoopThreadPool &&) = delete;

    EventLoop      *GetNextLoop() noexcept;
    const LoopList &GetAllLoops() const noexcept;

    void SetThreadNum(size_t num) noexcept {
        m_num_threads.store(num, std::memory_order_relaxed);
    }

    void Start() noexcept;

private:
    using WorkerList = std::vector<std::unique_ptr<EventLoopThread>>;

    std::atomic_bool   m_is_started;
    std::atomic_size_t m_num_threads;
    std::atomic_size_t m_next_loop;
    WorkerList         m_workers;
    LoopList           m_loops;
};

} // namespace eveio
