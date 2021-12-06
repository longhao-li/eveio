#ifndef EVEIO_QUEUE_HPP
#define EVEIO_QUEUE_HPP

#include "eveio/Vector.hpp"

#include <queue>

namespace eveio {

template <class T>
using Queue = std::queue<T, Vector<T>>;

template <class T, class Compare = std::less<T>>
using PriorityQueue = std::priority_queue<T, Vector<T>, Compare>;

} // namespace eveio

#endif // EVEIO_QUEUE_HPP
