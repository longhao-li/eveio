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

template <class T>
struct Constructor {
  template <class... Args>
  T *operator()(Args &&...args) {
    T *p = Allocator<T>::allocate(1);
    ::new (static_cast<void *>(p)) T(std::forward<Args>(args)...);
    return p;
  }
};

template <class T>
struct Destructor {
  void operator()(T *p) noexcept {
    p->~T();
    Allocator<T>::deallocate(p, 1);
  }
};

} // namespace eveio

#endif // EVEIO_ALLOCATOR_HPP
