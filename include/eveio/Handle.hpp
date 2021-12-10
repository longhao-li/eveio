#ifndef EVEIO_HANDLE_HPP
#define EVEIO_HANDLE_HPP

#include "eveio/Config.hpp"

#if defined(EVEIO_OS_WIN32)
#  include <handleapi.h>
#elif defined(EVEIO_HAS_POSIX)
#  include <unistd.h>
#else
#  error "unsupported operating system."
#endif

#if defined(EVEIO_OS_WIN32)
namespace eveio {

typedef void *native_handle_type;
static constexpr const native_handle_type INVALID_NATIVE_HANDLE =
    INVALID_HANDLE_VALUE;

inline bool close_handle(native_handle_type h) noexcept {
  return ::CloseHandle(h);
}

} // namespace eveio
#else
namespace eveio {

typedef int native_handle_type;
static constexpr const native_handle_type INVALID_NATIVE_HANDLE = -1;

inline bool handle_close(native_handle_type h) noexcept {
  return (::close(h) == 0);
}

} // namespace eveio
#endif

namespace eveio {

class Handle {
  native_handle_type h;

public:
  Handle() noexcept : h(INVALID_NATIVE_HANDLE) {}
  constexpr Handle(native_handle_type handle) noexcept : h(handle) {}

  constexpr Handle(const Handle &) noexcept = default;
  constexpr Handle &operator=(const Handle &) noexcept = default;

  ~Handle() noexcept = default;

  constexpr native_handle_type native_handle() const noexcept { return h; }

  static inline bool Close(Handle h) noexcept {
    return handle_close(h.native_handle());
  }

  constexpr operator native_handle_type() const noexcept { return h; }

  constexpr bool operator>(const Handle rhs) const noexcept {
    return h > rhs.h;
  }

  constexpr bool operator>=(const Handle rhs) const noexcept {
    return h >= rhs.h;
  }

  constexpr bool operator==(const Handle rhs) const noexcept {
    return h == rhs.h;
  }

  constexpr bool operator<(const Handle rhs) const noexcept {
    return h < rhs.h;
  }

  constexpr bool operator<=(const Handle rhs) const noexcept {
    return h <= rhs.h;
  }

  constexpr bool operator!=(const Handle rhs) const noexcept {
    return h != rhs.h;
  }
};

} // namespace eveio

#endif // EVEIO_HANDLE_HPP
