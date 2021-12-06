#include "eveio/WakeupHandle.hpp"
#include "eveio/Handle.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdlib>

using namespace eveio;

#if defined(EVEIO_OS_LINUX)
#elif defined(EVEIO_OS_DARWIN)

#  include <fcntl.h>
#  include <unistd.h>

static void SetCloexec(Handle h) noexcept {
  int old_flag = ::fcntl(h, F_GETFD);
  assert(old_flag >= 0);
  old_flag |= FD_CLOEXEC;
  ::fcntl(h, F_SETFD, old_flag);
}

static void SetNonblock(Handle h) noexcept {
  int old_flag = ::fcntl(h, F_GETFL);
  assert(old_flag >= 0);
  old_flag |= O_NONBLOCK;
  ::fcntl(h, F_SETFL, old_flag);
}

eveio::WakeupHandle::WakeupHandle() noexcept {
  int p[2]{};
  if (::pipe(p) < 0) {
    SPDLOG_CRITICAL("failed to create pipe: {}.", std::strerror(errno));
    std::abort();
  }
  fd[0] = p[0];
  fd[1] = p[1];
  SetNonblock(fd[0]);
  SetCloexec(fd[0]);
  SetNonblock(fd[1]);
  SetCloexec(fd[1]);
}

eveio::WakeupHandle::WakeupHandle(Handle _0, Handle _1) noexcept : fd{_0, _1} {
  SetNonblock(fd[0]);
  SetCloexec(fd[0]);
  SetNonblock(fd[1]);
  SetCloexec(fd[1]);
}

Result<WakeupHandle> eveio::WakeupHandle::Create() noexcept {
  int fd[2]{};
  if (::pipe(fd) < 0)
    return Result<WakeupHandle>::Error(std::strerror(errno));
  else
    return Result<WakeupHandle>::Ok(WakeupHandle(fd[0], fd[1]));
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
