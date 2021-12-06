#include "eveio/net/Buffer.hpp"
#include "eveio/net/Socket.hpp"

#include <cerrno>
#include <cstring>

using namespace eveio;
using namespace eveio::net;

static constexpr const int DefaultBufSize = 65536;

void eveio::net::Buffer::Append(const void *data, size_t byte) noexcept {
  storage.resize(tail + byte);
  memcpy(storage.data() + tail, data, byte);
  tail += byte;
}

int eveio::net::Buffer::ReadFromSocket(
    detail::native_socket_type sock) noexcept {
  char buf[DefaultBufSize]{};
  int tot_read = 0, byte_read = 0;

  for (;;) {
    byte_read = detail::socket_read(sock, buf, sizeof(buf));
    int saved_errno = errno;

    if (byte_read > 0) {
      Append(buf, byte_read);
      tot_read += byte_read;
      if (byte_read < DefaultBufSize)
        break;
    } else {
      if (byte_read < 0) {
        if (saved_errno == ECONNRESET)
          tot_read = byte_read;
        else
          errno = saved_errno;
      }
      break;
    }
  }

  return tot_read;
}
