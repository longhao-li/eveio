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

#include "eveio/Exception.hpp"

using namespace eveio;

static inline String ToString(size_t value) noexcept {
  return std::to_string(value);
}

eveio::Exception::Exception(const char *inType,
                            const char *inFile,
                            size_t inLine,
                            const char *inFunc,
                            String &&msg) noexcept
    : type(inType),
      file(inFile),
      line(inLine),
      func(inFunc),
      message(std::move(msg)),
      fullMessage() {
  if (file != nullptr && line > 0) {
    fullMessage.push_back('[');
    fullMessage.append(file);
    fullMessage.push_back(':');
    fullMessage.append(ToString(line));
    fullMessage.push_back(']');
  }

  if (type != nullptr) {
    fullMessage.push_back('[');
    fullMessage.append(type);
    fullMessage.push_back(']');
  }

  if (func != nullptr) {
    fullMessage.push_back('[');
    fullMessage.append(func);
    fullMessage.push_back(']');
  }

  fullMessage.push_back(' ');
  fullMessage.append(message);
}

eveio::Exception::Exception(const char *inType,
                            const char *inFile,
                            size_t inLine,
                            String &&msg) noexcept
    : Exception(inType, inFile, inLine, nullptr, std::move(msg)) {}

eveio::Exception::Exception(const char *inFile,
                            size_t inLine,
                            String &&msg) noexcept
    : Exception(nullptr, inFile, inLine, nullptr, std::move(msg)) {}

eveio::Exception::Exception(String &&msg) noexcept
    : Exception(nullptr, nullptr, 0, nullptr, std::move(msg)) {}
