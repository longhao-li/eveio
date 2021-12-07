#ifndef EVEIO_NET_BUFFER_HPP
#define EVEIO_NET_BUFFER_HPP

#include "eveio/String.hpp"
#include "eveio/Vector.hpp"
#include "eveio/net/Socket.hpp"

#include <memory>

namespace eveio {
namespace net {

class Buffer {
  Vector<char> storage;
  size_t head, tail;

public:
  Buffer() noexcept : storage(), head(0), tail(0) {}

  Buffer(const Buffer &) noexcept = default;
  Buffer &operator=(const Buffer &) noexcept = default;

  Buffer(Buffer &&) noexcept = default;
  Buffer &operator=(Buffer &&) noexcept = default;

  ~Buffer() noexcept = default;

  template <class T>
  T *Data() noexcept {
    return reinterpret_cast<T *>(&storage[0] + head);
  }

  template <class T>
  const T *Data() const noexcept {
    return reinterpret_cast<const T *>(&storage[0] + head);
  }

  void Clear() noexcept { head = tail = 0; }

  size_t Size() const noexcept { return (tail - head); }

  bool IsEmpty() const noexcept { return Size() == 0; }

  size_t Capacity() const noexcept { return (storage.capacity() - tail); }

  void Readout(size_t byte) noexcept {
    head += byte;
    if (head >= tail)
      Clear();
  }

  String RetrieveAsString() noexcept {
    String str(Data<char>(), Size());
    Clear();
    return str;
  }

  void Append(const void *data, size_t byte) noexcept;

  void Append(StringRef data) noexcept { Append(data.data(), data.size()); }

  // For internal usage. Do not call outside.
  bool ReadFromSocket(detail::native_socket_type sock, int &tot_read) noexcept;
};

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_BUFFER_HPP
