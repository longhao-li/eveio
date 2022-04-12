#pragma once

#include "eveio/Config.h"

namespace eveio {

#if EVEIO_OS_WIN32
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

    int GetListenHandle() const noexcept { return wakeupFd; }
};
#else
class WakeupHandle {
    int fd[2];

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

    int GetListenHandle() const noexcept { return fd[0]; }
};
#endif

} // namespace eveio
