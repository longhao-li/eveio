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

#ifndef EVEIO_HTTP_SERVER_HPP
#define EVEIO_HTTP_SERVER_HPP

#include "eveio/String.hpp"

#include <algorithm>
#include <functional>
#include <map>

namespace eveio {

namespace detail {

struct StringCaseInsensitiveLess {
  bool operator()(const String &a, const String &b) const noexcept {
    return std::lexicographical_compare(
        a.begin(), a.end(), b.begin(), b.end(), [](char l, char r) noexcept {
          return tolower(l) < tolower(r);
        });
  }
};

}; // namespace detail

using HttpHeaders =
    std::multimap<String, String, detail::StringCaseInsensitiveLess>;
using HttpParams = std::multimap<String, String>;

enum class HttpMethod {
  Undefined,
  Get,
  Head,
  Post,
  Put,
  Delete,
  Connect,
  Options,
  Trace,
  Patch,
};

class HttpRequest {
  HttpMethod method;
  String path;
  HttpHeaders headers;
  String body;

  uint8_t httpVersionMajor;
  uint8_t httpVersionMinor;

  String target;
  HttpParams params;

public:
  /// Parse HttpRequest from data.
  /// Throw InvalidParameterException if data is not valid HttpRequest.
  HttpRequest(const void *data, size_t len);
  ~HttpRequest() noexcept;

  HttpRequest(const HttpRequest &) noexcept;
  HttpRequest &operator=(const HttpRequest &) noexcept;

  HttpRequest(HttpRequest &&) noexcept;
  HttpRequest &operator=(HttpRequest &&) noexcept;

  bool HasHeader(const String &key) const noexcept;

  // Return nullptr if doesn't exists.
  const String *GetHeaderValue(const String &key, size_t id = 0) const noexcept;

  size_t GetHeaderValueCount(const String &key) const noexcept;

  /// A new header item will be created if key doesn't exist.
  void SetHeader(const String &key, const String &value) noexcept;
  /// A new header item will be created if key doesn't exist.
  void SetHeader(const String &key, String &&value) noexcept;
  /// A new header item will be created if key doesn't exist.
  void SetHeader(const String &key, StringRef value) noexcept;
  /// A new header item will be created if key doesn't exist.
  void SetHeader(const String &key, const char *value) noexcept;

  bool HasParam(const String &key) const noexcept;

  // Return nullptr if doesn't exists.
  const String *GetParamValue(const String &key, size_t id = 0) const noexcept;

  size_t GetParamValueCount(const String &key) const noexcept;
};

using HttpStatus = int;

const char *GetHttpStatusName(HttpStatus status) noexcept;

class HttpResponse {
  uint8_t httpVersionMajor;
  uint8_t httpVersionMinor;

  HttpStatus status;

  String resion;
  HttpHeaders headers;
  String body;
  String redirectTarget;

public:
  HttpResponse() noexcept;
  ~HttpResponse() noexcept;

  HttpResponse(const HttpResponse &) noexcept;
  HttpResponse &operator=(const HttpResponse &) noexcept;

  HttpResponse(HttpResponse &&) noexcept;
  HttpResponse &operator=(HttpResponse &&) noexcept;

  bool HasHeader(const String &key) const noexcept;

  /// Return nullptr if doesn't exists.
  const String *GetHeaderValue(const String &key, size_t id = 0) const noexcept;

  size_t GetHeaderValueCount(const String &key) const noexcept;

  /// A new header item will be created if key doesn't exist.
  void SetHeader(const String &key, const String &value) noexcept;
  /// A new header item will be created if key doesn't exist.
  void SetHeader(const String &key, String &&value) noexcept;
  /// A new header item will be created if key doesn't exist.
  void SetHeader(const String &key, StringRef value) noexcept;
  /// A new header item will be created if key doesn't exist.
  void SetHeader(const String &key, const char *value) noexcept;

  void SetRedirect(const String &url, HttpStatus status = 302) noexcept;
  void SetRedirect(String &&url, HttpStatus status = 302) noexcept;
  void SetRedirect(StringRef url, HttpStatus status = 302) noexcept;
  void SetRedirect(const char *url, HttpStatus status = 302) noexcept;
};

class HttpServer {
public:
  using Handler = std::function<void(const HttpRequest &, HttpResponse &)>;

  HttpServer();
  ~HttpServer() noexcept;

  HttpServer &Get(const String &pattern, Handler handler);
  HttpServer &Get(String &&pattern, Handler handler);
  HttpServer &Get(StringRef pattern, Handler handler);
  HttpServer &Get(const char *pattern, Handler handler);
  HttpServer &Post(const String &pattern, Handler handler);
  HttpServer &Post(String &&pattern, Handler handler);
  HttpServer &Post(StringRef pattern, Handler handler);
  HttpServer &Post(const char *pattern, Handler handler);
  HttpServer &Patch(const String &pattern, Handler handler);
  HttpServer &Patch(String &&pattern, Handler handler);
  HttpServer &Patch(StringRef pattern, Handler handler);
  HttpServer &Patch(const char *pattern, Handler handler);
  HttpServer &Delete(const String &pattern, Handler handler);
  HttpServer &Delete(String &&pattern, Handler handler);
  HttpServer &Delete(StringRef pattern, Handler handler);
  HttpServer &Delete(const char *pattern, Handler handler);
  HttpServer &Options(const String &pattern, Handler handler);
  HttpServer &Options(String &&pattern, Handler handler);
  HttpServer &Options(StringRef pattern, Handler handler);
  HttpServer &Options(const char *pattern, Handler handler);

private:
};

} // namespace eveio

#endif // EVEIO_HTTP_SERVER_HPP