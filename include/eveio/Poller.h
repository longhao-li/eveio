#pragma once

#include "eveio/Config.h"

#if EVEIO_OS_WIN32
#elif EVEIO_OS_LINUX

#    include "eveio/poller/EPollPoller.h"

namespace eveio {
using Poller = EPollPoller;
}

#elif EVEIO_OS_DARWIN || EVEIO_OS_FREEBSD

#    include "eveio/poller/KQueuePoller.h"

namespace eveio {
using Poller = KQueuePoller;
} // namespace eveio

#else
#    error "Unsupported operating system."
#endif
