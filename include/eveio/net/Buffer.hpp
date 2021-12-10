#ifndef EVEIO_NET_BUFFER_HPP
#define EVEIO_NET_BUFFER_HPP

#include "eveio/Allocator.hpp"
#include "eveio/String.hpp"
#include "eveio/Vector.hpp"
#include "eveio/net/Socket.hpp"

#include <cstring>
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
  bool ReadFromSocket(detail::native_socket_type sock, int64_t &tot_read) noexcept;
};

template <size_t Cap = 4096>
class BasicFixedBuffer {
  char p[Cap];
  size_t size;

public:
  static constexpr const size_t BufferCapacity = Cap;

  constexpr BasicFixedBuffer() noexcept : size(0) {}
  constexpr BasicFixedBuffer(const void *data, size_t sz) noexcept
      : p{}, size(sz) {
    std::memcpy(p, data, sz > Cap ? Cap : sz);
  }
  constexpr BasicFixedBuffer(const BasicFixedBuffer &other) noexcept
      : size(other.size) {
    std::memcpy(p, other.p, Cap);
  }
  constexpr BasicFixedBuffer &
  operator=(const BasicFixedBuffer &other) noexcept {
    p = other.p;
    size = other.size;
  }
  ~BasicFixedBuffer() noexcept = default;

  constexpr size_t Capacity() const noexcept { return Cap; }

  size_t Size() const noexcept { return size; }
  void SetSize(size_t sz) noexcept { size = sz; }

  char *Data() noexcept { return p; }
  const char *Data() const noexcept { return p; }

  template <class T>
  T *Data() noexcept {
    return reinterpret_cast<T *>(p);
  }

  template <class T>
  const T *Data() const noexcept {
    return reinterpret_cast<const T *>(p);
  }

  constexpr char operator[](size_t i) const noexcept { return p[i]; }
  constexpr char &operator[](size_t i) noexcept { return p[i]; }
};

typedef BasicFixedBuffer<4096> FixedBuffer;

} // namespace net
} // namespace eveio

#endif // EVEIO_NET_BUFFER_HPP
