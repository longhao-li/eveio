#include "eveio/EventLoopThread.hpp"

eveio::EventLoopThread::EventLoopThread() noexcept
    : loop(nullptr),
      loop_thread(),
      mutex(new std::mutex),
      cond(new std::condition_variable),
      init_callback() {}

eveio::EventLoopThread::EventLoopThread(eveio::EventLoopThread &&other) noexcept
    : loop(other.loop),
      loop_thread(std::move(other.loop_thread)),
      mutex(std::move(other.mutex)),
      cond(std::move(other.cond)),
      init_callback(std::move(other.init_callback)) {
  other.loop = nullptr;
}

eveio::EventLoopThread &
eveio::EventLoopThread::operator=(eveio::EventLoopThread &&other) noexcept {
  loop = other.loop;
  loop_thread = std::move(other.loop_thread);
  mutex = std::move(other.mutex);
  cond = std::move(other.cond);
  init_callback = std::move(other.init_callback);
  other.loop = nullptr;
  return (*this);
}

eveio::EventLoopThread::~EventLoopThread() noexcept {
  if (loop != nullptr) {
    loop->Quit();
    loop_thread.join();
  }
}

eveio::EventLoop *eveio::EventLoopThread::StartLoop() noexcept {
  loop_thread = std::thread(std::bind(&eveio::EventLoopThread::Task, this));
  std::unique_lock<std::mutex> lock(*mutex);
  cond->wait(lock, [this]() -> bool { return this->loop != nullptr; });
  return loop;
}

void eveio::EventLoopThread::Task() noexcept {
  EventLoop loop;
  if (init_callback)
    init_callback(&loop);

  {
    std::lock_guard<std::mutex> lock(*mutex);
    this->loop = &loop;
    cond->notify_one();
  }

  loop.Loop();

  std::lock_guard<std::mutex> lock(*mutex);
  this->loop = nullptr;
}
