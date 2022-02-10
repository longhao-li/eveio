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

#ifndef EVEIO_CONFIG_HPP
#define EVEIO_CONFIG_HPP

#if (defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(__NT__))
#  define EVEIO_OS_WIN32 1
#elif defined(__APPLE__)
#  define EVEIO_OS_DARWIN 1
#elif defined(__linux__) || defined(__linux)
#  define EVEIO_OS_LINUX 1
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#  define EVEIO_OS_FREEBSD 1
#else
#  error "Unsupported operating system."
#endif

#if EVEIO_OS_WIN32
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif

#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif

#  ifndef NOMINMAX
#    define NOMINMAX
#  endif

#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif

#  include <io.h>
#  include <winsock2.h>
#  include <ws2tcpip.h>

using handle_t = void *;

#else
#  include <arpa/inet.h>
#  include <fcntl.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>

using handle_t = int;
#endif

using socket_t = int;

#if defined(__clang__)
#  define EVEIO_COMPILER_CLANG 1
#elif defined(__GNUC__)
#  define EVEIO_COMPILER_GCC 1
#elif defined(_MSC_VER)
#  define EVEIO_COMPILER_MSVC 1
#else
#  error "Unsupported compiler."
#endif

#if EVEIO_COMPILER_CLANG
#  define __FILENAME__ __FILE_NAME__
#else
#  include <cstring>
#  if EVEIO_OS_WIN32
#    define __FILENAME__                                                       \
      (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#  else
#    define __FILENAME__                                                       \
      (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#  endif
#endif

#if (defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG))
#  define EVEIO_DEBUG 1
#endif

/// Thread ID
#include <cstddef>
#if EVEIO_OS_DARWIN || EVEIO_OS_FREEBSD
#  include <pthread.h>
#elif EVEIO_OS_LINUX
#  include <sys/syscall.h>
#  include <sys/types.h>
#endif

namespace eveio {

using threadid_t = size_t;

inline threadid_t __GetThreadID() noexcept {
#if EVEIO_OS_WIN32
  return static_cast<size_t>(::GetCurrentThreadId());
#elif EVEIO_OS_DARWIN
  uint64_t tid;
  ::pthread_threadid_np(nullptr, &tid);
  return static_cast<threadid_t>(tid);
#elif EVEIO_OS_LINUX
  return static_cast<threadid_t>(::syscall(SYS_gettid));
#elif EVEIO_OS_FREEBSD
  return static_cast<threadid_t>(::pthread_threadid_np());
#endif
}

inline threadid_t GetThreadID() noexcept {
  static thread_local const threadid_t tid = __GetThreadID();
  return tid;
}

} // namespace eveio

#endif // EVEIO_CONFIG_HPP
