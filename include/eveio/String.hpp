#ifndef EVEIO_STRING_HPP
#define EVEIO_STRING_HPP

#include "eveio/Allocator.hpp"
#include <string>
#include <string_view>

namespace eveio {

using String = std::basic_string<char, std::char_traits<char>, Allocator<char>>;
using StringRef = std::basic_string_view<char, std::char_traits<char>>;

} // namespace eveio

#endif // EVEIO_STRING_HPP
