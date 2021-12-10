#include "eveio/net/Buffer.hpp"
#include "eveio/net/Socket.hpp"

#include <cerrno>
#include <cstring>

using namespace eveio;
using namespace eveio::net;

static constexpr const int DEFAULT_BUFFER_SIZE = 65536;

void eveio::net::Buffer::Append(const void *data, size_t byte) noexcept {
  storage.reserve(tail + byte);
  memcpy(storage.data() + tail, data, byte);
  tail += byte;
}

bool eveio::net::Buffer::ReadFromSocket(native_socket_type sock,
                                        int64_t &tot_read) noexcept {
  char buf[DEFAULT_BUFFER_SIZE]{};
  tot_read = 0;
  int64_t byte_read = 0;

  for (;;) {
    byte_read = socket_read(sock, buf, sizeof(buf));

    if (byte_read > 0) {
      Append(buf, static_cast<size_t>(byte_read));
      tot_read += byte_read;
      if (byte_read < DEFAULT_BUFFER_SIZE)
        return true;
    } else {
      if (byte_read < 0)
        return false;
      return true;
    }
  }
}
