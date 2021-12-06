#include "eveio/Channel.hpp"
#include "eveio/Event.hpp"
#include "eveio/EventLoop.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

using namespace eveio;

eveio::Channel::Channel(EventLoop &loop, Handle hd) noexcept
    : loop(&loop),
      handle(hd),
      tied_object(),
      events_listening(event::NoneEvent),
      events_to_handle(event::NoneEvent),
      is_handling_event(false),
      is_added_to_loop(false),
      read_callback(),
      write_callback(),
      close_callback(),
      error_callback() {}

eveio::Channel::~Channel() noexcept {
  assert(!is_handling_event);
  assert(!is_added_to_loop);
}

bool eveio::Channel::IsRegisted() const noexcept {
  return loop->poller.HasChannel(*this);
}

void eveio::Channel::Update() noexcept {
  loop->poller.UpdateChannel(*this);
  is_added_to_loop = true;
}

void eveio::Channel::Unregist() noexcept {
  loop->poller.RemoveChannel(*this);
  is_added_to_loop = false;
}

void eveio::Channel::HandleEvent(Time recv_time) {
  if (events_to_handle == event::NoneEvent)
    return;

  std::shared_ptr<void> guard;
  if (is_tied) {
    guard = tied_object.lock();
    if (!guard) {
      SPDLOG_TRACE("tied object released.");
      return;
    }
  }

  is_handling_event = true;
  if ((events_to_handle & event::CloseEvent) &&
      !(events_to_handle & event::ReadEvent)) {
    if (close_callback)
      close_callback();
  }

  if (events_to_handle & event::ErrorEvent) {
    if (error_callback)
      error_callback();
  }

  if (events_to_handle & event::ReadEvent) {
    if (read_callback)
      read_callback(recv_time);
  }

  if (events_to_handle & event::WriteEvent) {
    if (write_callback)
      write_callback();
  }

  events_to_handle = event::NoneEvent;
  is_handling_event = false;
}
