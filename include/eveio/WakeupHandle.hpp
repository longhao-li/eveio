#ifndef EVEIO_WAKEUP_HANDLE_HPP
#define EVEIO_WAKEUP_HANDLE_HPP

#include "eveio/Handle.hpp"
#include "eveio/Result.hpp"

namespace eveio {

#if defined(EVEIO_OS_LINUX)
// use eventfd
#elif defined(EVEIO_OS_DARWIN)
// use pipe to simulate eventfd

class WakeupHandle {
  Handle fd[2];

  WakeupHandle(Handle _0, Handle _1) noexcept;

public:
  WakeupHandle() noexcept;
  static Result<WakeupHandle, const char *> Create() noexcept;
  static bool Close(WakeupHandle handle) noexcept;

  WakeupHandle(const WakeupHandle &) noexcept = default;
  WakeupHandle &operator=(const WakeupHandle &) noexcept = default;

  ~WakeupHandle() noexcept = default;

  void Trigger() const noexcept;
  bool Respond() const noexcept;

  Handle ListenHandle() const noexcept { return fd[0]; }

  operator Handle() const noexcept { return fd[0]; }

  bool operator<(const WakeupHandle rhs) const noexcept {
    return (fd[0] < rhs.fd[0] || (fd[0] == rhs.fd[0] && fd[1] < rhs.fd[1]));
  }

  bool operator==(const WakeupHandle rhs) const noexcept {
    return (fd[0] == rhs.fd[0]) && (fd[1] == rhs.fd[1]);
  }
};

#endif

} // namespace eveio

#endif // EVEIO_WAKEUP_HANDLE_HPP
