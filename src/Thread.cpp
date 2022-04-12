#include "eveio/Thread.h"
#include "eveio/Config.h"

#if EVEIO_OS_LINUX
#    include <sys/syscall.h>
#endif

using namespace eveio;

static thread_id_t GetThreadIDImpl() {
#if EVEIO_OS_WIN32
    return static_cast<thread_id_t>(::GetCurrentThreadId());
#elif EVEIO_OS_DARWIN
    uint64_t tid;
    ::pthread_threadid_np(nullptr, &tid);
    return static_cast<thread_id_t>(tid);
#elif EVEIO_OS_LINUX
    return static_cast<thread_id_t>(::syscall(SYS_gettid));
#elif EVEIO_OS_FREEBSD
    return static_cast<thread_id_t>(::pthread_threadid_np());
#endif
}

thread_id_t eveio::GetThreadID() noexcept {
    static thread_local const thread_id_t tid = GetThreadIDImpl();
    return tid;
}
