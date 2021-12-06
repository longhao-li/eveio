#ifndef EVEIO_ALLOCATOR_HPP
#define EVEIO_ALLOCATOR_HPP

#include <mimalloc.h>

#include <memory>

namespace eveio {

/// mimalloc allocator
template <class T>
class Allocator {
public:
  typedef T value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef std::true_type propagate_on_container_move_assignment;

  Allocator() noexcept = default;
  Allocator(const Allocator &) noexcept = default;
  Allocator &operator=(const Allocator &) noexcept = default;
  Allocator(Allocator &&) noexcept = default;
  Allocator &operator=(Allocator &&) noexcept = default;
  ~Allocator() noexcept = default;

  static constexpr T *allocate(size_t n) noexcept {
    return static_cast<T *>(mi_malloc(n * sizeof(T)));
  }

  static constexpr void deallocate(T *p, size_t) noexcept { mi_free(p); }
};

} // namespace eveio

#endif // EVEIO_ALLOCATOR_HPP
