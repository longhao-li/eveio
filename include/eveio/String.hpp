/// Copyright (c) 2021 Li Longhao
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.

#ifndef EVEIO_STRING_HPP
#define EVEIO_STRING_HPP

#include <algorithm>
#include <limits>
#include <regex>
#include <stdexcept>
#include <string>

namespace eveio {

using String = std::string;

// std::string_view
class StringRef {
public:
  using traits_type = std::char_traits<char>;
  using value_type = char;
  using pointer = char *;
  using const_pointer = const char *;
  using reference = char &;
  using const_reference = const char &;
  using const_iterator = const_pointer;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  static constexpr const size_type npos = size_type(-1);

  constexpr StringRef() noexcept = default;
  constexpr StringRef(const StringRef &) noexcept = default;
  StringRef &operator=(const StringRef &) noexcept = default;

  constexpr StringRef(const char *s, size_type len) noexcept
      : ptr(s), sz(len) {}

  constexpr StringRef(const char *s)
      : ptr(s), sz(s ? traits_type::length(s) : 0) {}

  constexpr const_iterator cbegin() const noexcept { return ptr; }
  constexpr const_iterator cend() const noexcept { return ptr + sz; }
  constexpr const_iterator begin() const noexcept { return cbegin(); }
  constexpr const_iterator end() const noexcept { return cend(); }

  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(cend());
  }
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(cbegin());
  }

  const_reverse_iterator rbegin() const noexcept { return crbegin(); }
  const_reverse_iterator rend() const noexcept { return crend(); }

  constexpr size_type size() const noexcept { return sz; }
  constexpr size_type length() const noexcept { return size(); }

  constexpr size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max();
  }

  constexpr bool empty() const noexcept { return size() == 0; }

  constexpr const_reference operator[](size_type pos) const noexcept {
    return ptr[pos];
  }

  const_reference at(size_type pos) const {
    if (pos >= size()) {
      throw std::out_of_range("StringRef::at");
    }
    return ptr[pos];
  }

  constexpr const_reference front() const noexcept { return ptr[0]; }
  constexpr const_reference back() const noexcept { return ptr[sz - 1]; }

  constexpr const_pointer data() const noexcept { return ptr; }

  int compare(StringRef other) const noexcept {
    size_type len = std::min(size(), other.size());
    int ret = traits_type::compare(ptr, other.ptr, len);
    if (ret == 0) {
      ret = (size() == other.size() ? 0 : (size() < other.size() ? -1 : 1));
    }
    return ret;
  }

  int compare(const char *s) const noexcept { return compare(StringRef(s)); }

private:
  const value_type *ptr = nullptr;
  size_type sz = 0;
};

inline bool operator==(StringRef lhs, StringRef rhs) noexcept {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  return lhs.compare(rhs) == 0;
}

inline bool operator!=(StringRef lhs, StringRef rhs) noexcept {
  return !(lhs == rhs);
}

inline bool operator<(StringRef lhs, StringRef rhs) noexcept {
  return lhs.compare(rhs) < 0;
}

inline bool operator>(StringRef lhs, StringRef rhs) noexcept {
  return lhs.compare(rhs) > 0;
}

inline bool operator<=(StringRef lhs, StringRef rhs) noexcept {
  return lhs.compare(rhs) <= 0;
}

inline bool operator>=(StringRef lhs, StringRef rhs) noexcept {
  return lhs.compare(rhs) >= 0;
}

} // namespace eveio

#endif // EVEIO_STRING_HPP
