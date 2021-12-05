#include "eveio/WakeupHandle.hpp"
#include "eveio/Handle.hpp"

#include <cassert>

using eveio::Result;
using eveio::WakeupHandle;

#if defined(EVEIO_OS_LINUX)
#elif defined(EVEIO_OS_DARWIN)

#  include <fcntl.h>
#  include <unistd.h>

static void SetCloexec(eveio::Handle h) noexcept {
  int old_flag = ::fcntl(h, F_GETFD);
  assert(old_flag >= 0);
  old_flag |= FD_CLOEXEC;
  ::fcntl(h, F_SETFD, old_flag);
}

static void SetNonblock(eveio::Handle h) noexcept {
  int old_flag = ::fcntl(h, F_GETFL);
  assert(old_flag >= 0);
  old_flag |= O_NONBLOCK;
  ::fcntl(h, F_SETFL, old_flag);
}

eveio::WakeupHandle::WakeupHandle(eveio::Handle _0, eveio::Handle _1) noexcept
    : fd{_0, _1} {
  SetNonblock(fd[0]);
  SetCloexec(fd[0]);
  SetNonblock(fd[1]);
  SetCloexec(fd[1]);
}

Result<WakeupHandle, const char *> eveio::WakeupHandle::Create() noexcept {
  int fd[2]{};
  if (::pipe(fd) < 0)
    return Result<WakeupHandle, const char *>::Error(std::strerror(errno));
  else
    return Result<WakeupHandle, const char *>::Ok(WakeupHandle(fd[0], fd[1]));
}

bool eveio::WakeupHandle::Close(WakeupHandle handle) noexcept {
  bool res = true;
  res = (Handle::Close(handle.fd[0]) && res);
  res = (Handle::Close(handle.fd[1]) && res);
  return res;
}

void eveio::WakeupHandle::Trigger() const noexcept {
  int val = 1;
  ::write(fd[1], &val, sizeof(val));
}

bool eveio::WakeupHandle::Respond() const noexcept {
  int val = 0;
  ::read(fd[0], &val, sizeof(val));
  return val == 1;
}

#endif
