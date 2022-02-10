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

#include "eveio/WakeupHandle.hpp"
#include "eveio/Exception.hpp"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>

using namespace eveio;

#if EVEIO_OS_WIN32
#elif EVEIO_OS_DARWIN
#  include <fcntl.h>
#  include <unistd.h>

static void SetCloexec(handle_t h) noexcept {
  int oldFlag = ::fcntl(h, F_GETFD);
  assert(oldFlag >= 0);
  oldFlag |= FD_CLOEXEC;
  ::fcntl(h, F_SETFD, oldFlag);
}

static void SetNonblock(handle_t h) noexcept {
  int oldFlag = ::fcntl(h, F_GETFL);
  assert(oldFlag >= 0);
  oldFlag |= O_NONBLOCK;
  ::fcntl(h, F_SETFL, oldFlag);
}

eveio::WakeupHandle::WakeupHandle() : fd{-1, -1} {
  if (::pipe(fd) < 0) {
    throw SystemErrorException(
        __FILENAME__, __LINE__, __func__, std::strerror(errno));
  }

  SetCloexec(fd[0]);
  SetCloexec(fd[1]);
  SetNonblock(fd[0]);
  SetNonblock(fd[1]);
}

eveio::WakeupHandle::~WakeupHandle() noexcept {
  ::close(fd[0]);
  ::close(fd[1]);
}

void eveio::WakeupHandle::Trigger() const noexcept {
  uint64_t value = 1;
  ::write(fd[1], &value, sizeof(value));
}

bool eveio::WakeupHandle::Respond() const noexcept {
  uint64_t value = 0;
  ::read(fd[0], &value, sizeof(value));
  return (value == 1);
}

#elif EVEIO_OS_LINUX || EVEIO_OS_FREEBSD
#  include <sys/eventfd.h>

eveio::WakeupHandle::WakeupHandle()
    : wakeupFd(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) {
  if (wakeupFd < 0) {
    throw SystemErrorException(__FILENAME__,
                               __LINE__,
                               __func__,
                               String("Failed to create eventfd: ") +
                                   std::strerror(errno));
  }
}

eveio::WakeupHandle::~WakeupHandle() noexcept { ::close(wakeupFd); }

void eveio::WakeupHandle::Trigger() const noexcept {
  uint64_t value = 1;
  ::write(wakeupFd, &value, sizeof(value));
}

bool eveio::WakeupHandle::Respond() const noexcept {
  uint64_t value = 0;
  ::read(wakeupFd, &value, sizeof(value));
  return (value == 1);
}

#endif
