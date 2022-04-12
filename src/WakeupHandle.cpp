#include "eveio/WakeupHandle.h"

#include <cassert>
#include <cerrno>

using namespace eveio;

#if EVEIO_OS_WIN32
#elif EVEIO_OS_LINUX || EVEIO_OS_FREEBSD
#    include <sys/eventfd.h>

eveio::WakeupHandle::WakeupHandle()
    : wakeupFd(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) {
    assert(wakeupFd >= 0);
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
#else
static void SetCloexec(int h) noexcept {
    int oldFlag = ::fcntl(h, F_GETFD);
    assert(oldFlag >= 0);
    oldFlag |= FD_CLOEXEC;
    ::fcntl(h, F_SETFD, oldFlag);
}

static void SetNonblock(int h) noexcept {
    int oldFlag = ::fcntl(h, F_GETFL);
    assert(oldFlag >= 0);
    oldFlag |= O_NONBLOCK;
    ::fcntl(h, F_SETFL, oldFlag);
}

eveio::WakeupHandle::WakeupHandle() : fd{-1, -1} {
    int ret = ::pipe(fd);
    assert(ret >= 0);
    (void)ret;

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
#endif
