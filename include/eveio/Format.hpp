#ifndef EVEIO_FORMAT_HPP
#define EVEIO_FORMAT_HPP

#include "eveio/Allocator.hpp"
#include "eveio/String.hpp"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <iterator>

namespace eveio {

template <class... Args>
String Format(fmt::string_view format, Args &&...args) {
  using Buffer =
      fmt::basic_memory_buffer<char, fmt::inline_buffer_size, Allocator<char>>;

  Allocator<char> alloc;
  Buffer buf(alloc);
  fmt::vformat_to(
      std::back_inserter(buf),
      format,
      fmt::make_args_checked<Args...>(format, std::forward<Args>(args)...));
  return String(buf.data(), buf.size(), alloc);
}

} // namespace eveio

#endif // EVEIO_FORMAT_HPP
