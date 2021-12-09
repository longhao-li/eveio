#include "eveio/net/Buffer.hpp"
#include "eveio/net/Socket.hpp"

#include <cerrno>
#include <cstring>

using namespace eveio;
using namespace eveio::net;

static constexpr const int DefaultBufSize = 65536;

void eveio::net::Buffer::Append(const void *data, size_t byte) noexcept {
  storage.reserve(tail + byte);
  memcpy(storage.data() + tail, data, byte);
  tail += byte;
}

bool eveio::net::Buffer::ReadFromSocket(detail::native_socket_type sock,
                                        int &tot_read) noexcept {
  char buf[DefaultBufSize]{};
  tot_read = 0;
  int64_t byte_read = 0;

  for (;;) {
    byte_read = detail::socket_read(sock, buf, sizeof(buf));

    if (byte_read > 0) {
      Append(buf, static_cast<size_t>(byte_read));
      tot_read += byte_read;
      if (byte_read < DefaultBufSize)
        return true;
    } else {
      if (byte_read < 0)
        return false;
      return true;
    }
  }
}
