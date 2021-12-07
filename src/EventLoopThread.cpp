#include "eveio/EventLoopThread.hpp"
#include <cassert>

using namespace eveio;

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
  EventLoop loop;
  if (init_callback)
    init_callback(&loop);

  {
    std::lock_guard<std::mutex> lock(mutex);
    this->loop = &loop;
    cond.notify_one();
  }

  loop.Loop();

  std::lock_guard<std::mutex> lock(mutex);
  this->loop = nullptr;
}
