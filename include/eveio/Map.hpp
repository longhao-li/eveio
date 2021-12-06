#ifndef EVEIO_MAP_HPP
#define EVEIO_MAP_HPP

#include "eveio/Allocator.hpp"

#include <map>
#include <unordered_map>

namespace eveio {

template <class Key, class Value, class Compare = std::less<Key>>
using Map =
    std::map<Key, Value, Compare, Allocator<std::pair<const Key, Value>>>;

template <class Key, class Value, class Compare = std::less<Key>>
using MultiMap =
    std::multimap<Key, Value, Compare, Allocator<std::pair<const Key, Value>>>;

template <class Key,
          class Value,
          class Hash = std::hash<Key>,
          class Pred = std::equal_to<Key>>
using HashMap = std::unordered_map<Key,
                                   Value,
                                   Hash,
                                   Pred,
                                   Allocator<std::pair<const Key, Value>>>;

} // namespace eveio

#endif // EVEIO_MAP_HPP
