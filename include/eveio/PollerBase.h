#pragma once

#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace eveio {

class Listener;

/// It is possible that the derived class may throw exceptions. This usually
/// happens when the poller failed to create its handle.
///
/// For example, KQueuePoller will throw PollerException if it failed to create
/// kqueue file descriptor.
template <typename T>
class PollerBase {
protected:
    PollerBase() noexcept = default;
    ~PollerBase()         = default;

public:
    PollerBase(const PollerBase &) = delete;
    PollerBase &operator=(const PollerBase &) = delete;

    PollerBase(PollerBase &&) = delete;
    PollerBase &operator=(PollerBase &&) = delete;

    void Poll(std::chrono::milliseconds timeout) {
        static_cast<T *>(this)->Poll(timeout);
    }

    void UpdateListener(Listener &listener) {
        static_cast<T *>(this)->UpdateListener(listener);
    }

    void UnregistListener(Listener &listener) {
        static_cast<T *>(this)->UnregistListener(listener);
    }
};

} // namespace eveio
