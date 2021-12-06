#ifndef EVEIO_NET_CONFIG_HPP
#define EVEIO_NET_CONFIG_HPP

#include "eveio/Config.hpp"

#if defined(EVEIO_OS_WIN32)
#  include <Ws2tcpip.h>
#else
#  include <arpa/inet.h>
#  include <fcntl.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <unistd.h>
#endif

#endif // EVEIO_NET_CONFIG_HPP
