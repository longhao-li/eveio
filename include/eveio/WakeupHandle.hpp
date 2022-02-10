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

#ifndef EVEIO_WAKEUP_HANDLE_HPP
#define EVEIO_WAKEUP_HANDLE_HPP

#include "eveio/Config.hpp"

namespace eveio {
#if EVEIO_OS_WIN32
#  error "To be implemented for Windows."
#elif EVEIO_OS_DARWIN
/// Use pipe to simulate wakeup fd in linux.
class WakeupHandle {
  handle_t fd[2];

public:
  /// Throw SystemErrorException if failed to create pipe fd.
  WakeupHandle();

  /// Close automatically.
  ~WakeupHandle() noexcept;

  WakeupHandle(const WakeupHandle &) = delete;
  WakeupHandle &operator=(const WakeupHandle &) = delete;

  WakeupHandle(WakeupHandle &&) = delete;
  WakeupHandle &operator=(WakeupHandle &&) = delete;

  /// Wakeup. Write some data to fd[1].
  void Trigger() const noexcept;

  /// Respond to Trigger(). Read some data from fd[0].
  bool Respond() const noexcept;

  handle_t GetListenHandle() const noexcept { return fd[0]; }
};
#elif EVEIO_OS_LINUX || EVEIO_OS_FREEBSD
class WakeupHandle {
  int wakeupFd;

public:
  /// Throw SystemErrorException if failed to create wakeup fd.
  WakeupHandle();

  /// Close automatically.
  ~WakeupHandle() noexcept;

  WakeupHandle(const WakeupHandle &) = delete;
  WakeupHandle &operator=(const WakeupHandle &) = delete;

  WakeupHandle(WakeupHandle &&) = delete;
  WakeupHandle &operator=(WakeupHandle &&) = delete;

  /// Wakeup. Write some data to wakeup fd.
  void Trigger() const noexcept;

  /// Respond to Trigger().
  bool Respond() const noexcept;

  handle_t GetListenHandle() const noexcept { return wakeupFd; }
};
#endif
} // namespace eveio

#endif // EVEIO_WAKEUP_HANDLE_HPP
