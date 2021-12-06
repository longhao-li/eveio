#ifndef EVEIO_VECTOR_HPP
#define EVEIO_VECTOR_HPP

#include "eveio/Allocator.hpp"

#include <vector>

namespace eveio {

template <class T>
using Vector = std::vector<T, Allocator<T>>;

}

#endif // EVEIO_VECTOR_HPP
