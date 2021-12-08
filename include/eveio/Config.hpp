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
#endif

#if defined(EVEIO_OS_LINUX)
#  define EVEIO_POLLER_HAS_EPOLL 1
#endif

#if defined(EVEIO_OS_DARWIN) || defined(EVEIO_OS_FREEBSD)
#  define EVEIO_POLLER_HAS_KQUEUE 1
#endif

#endif // EVEIO_CONFIG_HPP
