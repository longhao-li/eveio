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

#include "eveio/Channel.hpp"
#include "eveio/Event.hpp"
#include "eveio/Eventloop.hpp"
#include "eveio/Exception.hpp"
#include "eveio/Poller.hpp"

#include <cassert>

using namespace eveio;

eveio::Channel::Channel(Eventloop &ownerLoop, handle_t fd) noexcept
    : loop(&ownerLoop),
      handle(fd),
      isTied(false),
      isHandlingEvent(false),
      isAddedToLoop(false),
      tiedObject(),
      eventsListing(EVENT_NONE),
      eventsToHandle(EVENT_NONE),
      pollerState(Poller::ChannelInitPollState),
      readCallback(),
      writeCallback(),
      errorCallback(),
      closeCallback() {}

eveio::Channel::~Channel() noexcept {
  assert(!isHandlingEvent);
  assert(!isAddedToLoop);
  (void)0;
}

void eveio::Channel::Update() {
  loop->UpdateChannel(this);
  isAddedToLoop = true;
}

void eveio::Channel::Unregist() {
  loop->UnregistChannel(this);
  isAddedToLoop = false;
}

void eveio::Channel::HandleEvent(
    std::chrono::system_clock::time_point recvTime) {
  if (eventsToHandle == EVENT_NONE) {
    return;
  }

  std::shared_ptr<void> guard;
  if (isTied) {
    guard = tiedObject.lock();
    if (!guard) {
      return;
    }
  }

  isHandlingEvent = true;
  if ((eventsToHandle & EVENT_CLOSE)) {
    if (closeCallback) {
      closeCallback();
    }
  }

  if (eventsToHandle & EVENT_ERROR) {
    if (errorCallback) {
      errorCallback();
    }
  }

  if (eventsToHandle & EVENT_READ) {
    if (readCallback) {
      readCallback(recvTime);
    }
  }

  if (eventsToHandle & EVENT_WRITE) {
    if (writeCallback) {
      writeCallback();
    }
  }

  eventsToHandle = EVENT_NONE;
  isHandlingEvent = false;
}
