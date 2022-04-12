#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

namespace eveio {

class EventLoop;

class EventLoopThread {
public:
    EventLoopThread() noexcept;
    ~EventLoopThread();

    EventLoopThread(const EventLoopThread &) = delete;
    EventLoopThread &operator=(const EventLoopThread &) = delete;

    EventLoopThread(EventLoopThread &&) = delete;
    EventLoopThread &operator=(EventLoopThread &&) = delete;

    EventLoop *StartLoop() noexcept;

    EventLoop *GetLoop() const noexcept { return m_loop; }

private:
    void Task() noexcept;

private:
    EventLoop              *m_loop;
    std::thread             m_loop_thread;
    std::mutex              m_loop_mutex;
    std::condition_variable m_loop_cond;
};

} // namespace eveio
