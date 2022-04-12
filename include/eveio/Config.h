#pragma once

#if (defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(__NT__))
#    define EVEIO_OS_WIN32 1
#elif defined(__APPLE__)
#    define EVEIO_OS_DARWIN 1
#elif defined(__linux__) || defined(__linux)
#    define EVEIO_OS_LINUX 1
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#    define EVEIO_OS_FREEBSD 1
#else
#    error "Unsupported operating system."
#endif

#if EVEIO_OS_WIN32
#    ifndef _CRT_SECURE_NO_WARNINGS
#        define _CRT_SECURE_NO_WARNINGS
#    endif

#    ifndef _CRT_NONSTDC_NO_DEPRECATE
#        define _CRT_NONSTDC_NO_DEPRECATE
#    endif

#    ifndef NOMINMAX
#        define NOMINMAX
#    endif

#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif

#    include <io.h>
#    include <winsock2.h>
#    include <ws2tcpip.h>
#else
#    include <arpa/inet.h>
#    include <fcntl.h>
#    include <netinet/in.h>
#    include <netinet/tcp.h>
#    include <pthread.h>
#    include <sys/socket.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif
