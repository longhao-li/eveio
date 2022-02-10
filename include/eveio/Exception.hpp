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

#ifndef EVEIO_EXCEPTION_HPP
#define EVEIO_EXCEPTION_HPP

#include "eveio/String.hpp"

#include <cstddef>
#include <exception>

namespace eveio {

class Exception : public std::exception {
  const char *type;
  const char *file;
  size_t line;
  const char *func;
  String message;
  String fullMessage;

public:
  Exception(const char *inType,
            const char *inFile,
            size_t inLine,
            const char *inFunc,
            String &&msg) noexcept;

  Exception(const char *inType,
            const char *inFile,
            size_t inLine,
            String &&msg) noexcept;

  Exception(const char *inFile, size_t inLine, String &&msg) noexcept;

  explicit Exception(String &&msg) noexcept;

  ~Exception() noexcept override = default;

  Exception(const Exception &) noexcept = default;
  Exception &operator=(const Exception &) noexcept = default;

  Exception(Exception &&) noexcept = default;
  Exception &operator=(Exception &&) noexcept = default;

  const char *GetType() const noexcept { return type; }
  const char *GetFile() const noexcept { return file; }
  size_t GetLine() const noexcept { return line; }
  const char *GetFunc() const noexcept { return func; }
  const String &GetMessage() const noexcept { return message; }
  const String &GetFullMessage() const noexcept { return fullMessage; }

  const char *what() const noexcept override { return fullMessage.c_str(); }
};

class UnimplementedException : public Exception {
public:
  UnimplementedException(const char *inFile,
                         size_t inLine,
                         String &&msg) noexcept
      : Exception(__func__, inFile, inLine, std::move(msg)) {}

  UnimplementedException(const char *inFile,
                         size_t inLine,
                         const char *inFunc,
                         String &&msg) noexcept
      : Exception(__func__, inFile, inLine, inFunc, std::move(msg)) {}
};

class InvalidParameterException : public Exception {
public:
  InvalidParameterException(const char *inFile,
                            size_t inLine,
                            String &&msg) noexcept
      : Exception(__func__, inFile, inLine, std::move(msg)) {}

  InvalidParameterException(const char *inFile,
                            size_t inLine,
                            const char *inFunc,
                            String &&msg) noexcept
      : Exception(__func__, inFile, inLine, inFunc, std::move(msg)) {}
};

class SystemErrorException : public Exception {
public:
  SystemErrorException(const char *inFile, size_t inLine, String &&msg) noexcept
      : Exception(__func__, inFile, inLine, std::move(msg)) {}

  SystemErrorException(const char *inFile,
                       size_t inLine,
                       const char *inFunc,
                       String &&msg) noexcept
      : Exception(__func__, inFile, inLine, inFunc, std::move(msg)) {}
};

class RuntimeErrorException : public Exception {
public:
  RuntimeErrorException(const char *inFile,
                        size_t inLine,
                        String &&msg) noexcept
      : Exception(__func__, inFile, inLine, std::move(msg)) {}

  RuntimeErrorException(const char *inFile,
                        size_t inLine,
                        const char *inFunc,
                        String &&msg) noexcept
      : Exception(__func__, inFile, inLine, inFunc, std::move(msg)) {}
};

} // namespace eveio

#endif // EVEIO_EXCEPTION_HPP
