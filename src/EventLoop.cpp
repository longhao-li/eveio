#include "eveio/EventLoop.h"
#include "eveio/Listener.h"

using namespace eveio;

eveio::EventLoop::EventLoop()
    : m_poller(),
      m_is_looping(false),
      m_is_quit(false),
      m_thread_id(GetThreadID()),
      m_wakeup_handle(),
      m_wakeup_listener(new Listener(*this, m_wakeup_handle.GetListenHandle())),
      m_pending_func(),
      m_pending_func_mutex() {
    m_wakeup_listener->TieObject(this);
    m_wakeup_listener->SetReadCallback([](Listener *listener) {
        auto loop = static_cast<EventLoop *>(listener->GetTiedObject());
        loop->m_wakeup_handle.Respond();
    });
    m_wakeup_listener->EnableReading();
}

eveio::EventLoop::~EventLoop() {
    m_wakeup_listener->DisableAll();
    m_wakeup_listener->Unregister();
}

void eveio::EventLoop::Loop() {
    // Flush memory
    if (m_is_looping.exchange(true, std::memory_order_relaxed))
        return;

    while (!m_is_quit.load(std::memory_order_relaxed)) {
        m_poller.Poll(std::chrono::milliseconds(10000));
        // Do pending functions.
        std::vector<std::function<void()>> pendingFuncCopy;
        {
            std::lock_guard<std::mutex> guard(m_pending_func_mutex);
            pendingFuncCopy.swap(m_pending_func);
        }

        for (const auto &func : pendingFuncCopy) {
            func();
        }
    }

    m_is_looping.exchange(false, std::memory_order_relaxed);
}
