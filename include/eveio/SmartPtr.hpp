#ifndef EVEIO_SMARTPTR_HPP
#define EVEIO_SMARTPTR_HPP

#include "eveio/Allocator.hpp"

#include <memory>

namespace eveio {

template <class T, class Dp = Destructor<T>>
using UniquePtr = std::unique_ptr<T, Dp>;

template <class T, class... Args>
UniquePtr<T> MakeUnique(Args &&...args) {
  return UniquePtr<T>(Constructor<T>{}(std::forward<Args>(args)...),
                      Destructor<T>{});
}

template <class T>
using SharedPtr = std::shared_ptr<T>;

template <class T, class... Args>
SharedPtr<T> MakeShared(Args &&...args) {
  return SharedPtr<T>(Constructor<T>{}(std::forward<Args>(args)...),
                      Destructor<T>{});
}

template <class T>
using WeakPtr = std::weak_ptr<T>;

} // namespace eveio

#endif // EVEIO_SMARTPTR_HPP
