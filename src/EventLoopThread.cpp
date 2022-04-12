#include "eveio/EventLoopThread.h"
#include "eveio/EventLoop.h"

using namespace eveio;

eveio::EventLoopThread::EventLoopThread() noexcept
    : m_loop(nullptr), m_loop_thread(), m_loop_mutex(), m_loop_cond() {}

eveio::EventLoopThread::~EventLoopThread() {
    if (m_loop != nullptr) {
        m_loop->Quit();
        m_loop_thread.join();
    }
}

EventLoop *eveio::EventLoopThread::StartLoop() noexcept {
    m_loop_thread = std::thread([this]() { this->Task(); });
    std::unique_lock<std::mutex> guard(m_loop_mutex);
    m_loop_cond.wait(guard,
                     [this]() -> bool { return this->m_loop != nullptr; });
    return m_loop;
}

void eveio::EventLoopThread::Task() noexcept {
    EventLoop task_loop;

    {
        std::lock_guard<std::mutex> guard(m_loop_mutex);
        m_loop = &task_loop;
        m_loop_cond.notify_one();
    }

    task_loop.Loop();

    std::lock_guard<std::mutex> guard(m_loop_mutex);
    m_loop = nullptr;
}
