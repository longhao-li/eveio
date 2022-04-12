#pragma once

#include <cstdint>

namespace eveio {

using thread_id_t = uint64_t;

thread_id_t GetThreadID() noexcept;

} // namespace eveio
