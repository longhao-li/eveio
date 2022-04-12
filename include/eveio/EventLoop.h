#pragma once

#include "eveio/Poller.h"
#include "eveio/Thread.h"
#include "eveio/WakeupHandle.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

namespace eveio {

class Listener;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    EventLoop(EventLoop &&) = delete;
    EventLoop &operator=(EventLoop &&) = delete;

    void Loop();

    void Quit() {
        m_is_quit.store(true, std::memory_order_relaxed);
        if (!IsInLoopThread()) {
            WakeUp();
        }
    }

    void WakeUp() const noexcept { m_wakeup_handle.Trigger(); }

    bool IsLooping() const noexcept {
        return m_is_looping.load(std::memory_order_relaxed);
    }

    bool IsInLoopThread() const noexcept {
        return GetThreadID() == m_thread_id;
    }

    thread_id_t GetLoopThreadId() const noexcept { return m_thread_id; }

    template <typename Fn>
    void RunInLoop(Fn &&fn) {
        if (IsInLoopThread()) {
            fn();
        } else {
            QueueInLoop(std::function<void()>(std::forward<Fn>(fn)));
            WakeUp();
        }
    }

    void QueueInLoop(std::function<void()> fn) {
        static_assert(
            std::is_constructible<std::function<void()>, decltype(fn)>::value,
            "Function must be constructible to std::function<void()>.");

        std::lock_guard<std::mutex> guard(this->m_pending_func_mutex);
        m_pending_func.push_back(std::move(fn));
    }

    /// For internal usage. Do not call this method manually.
    void UpdateListener(Listener &listener) {
        m_poller.UpdateListener(listener);
    }

    /// For internal usage. Do not call this method manually.
    void UnregistListener(Listener &listener) {
        m_poller.UnregistListener(listener);
    }

private:
    Poller           m_poller;
    std::atomic_bool m_is_looping;
    std::atomic_bool m_is_quit;

    const thread_id_t m_thread_id;

    WakeupHandle              m_wakeup_handle;
    std::unique_ptr<Listener> m_wakeup_listener;

    std::vector<std::function<void()>> m_pending_func;
    mutable std::mutex                 m_pending_func_mutex;
};

} // namespace eveio
