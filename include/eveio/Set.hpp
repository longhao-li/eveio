#ifndef EVEIO_SET_HPP
#define EVEIO_SET_HPP

#include "eveio/Allocator.hpp"

#include <set>
#include <unordered_set>

namespace eveio {

template <class T, class Compare = std::less<T>>
using Set = std::set<T, Compare, Allocator<T>>;

template <class T, class Compare = std::less<T>>
using MultiSet = std::multiset<T, Compare, Allocator<T>>;

template <class T, class Hash = std::hash<T>, class Pred = std::equal_to<T>>
using HashSet = std::unordered_set<T, Hash, Pred, Allocator<T>>;

} // namespace eveio

#endif // EVEIO_SET_HPP
