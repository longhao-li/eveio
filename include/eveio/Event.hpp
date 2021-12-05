#ifndef EVEIO_EVENT_HPP
#define EVEIO_EVENT_HPP

#include <cstdint>

namespace eveio {
namespace event {

enum {
  NoneEvent = 0x0,
  ReadEvent = 0x1,
  WriteEvent = 0x2,
  CloseEvent = 0x4,
  ErrorEvent = 0x8,
};

}

typedef uint32_t Events;

} // namespace eveio

#endif // EVEIO_EVENT_HPP
