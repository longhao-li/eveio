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

#include "eveio/HttpServer.hpp"
#include "eveio/AsyncTcpConnection.hpp"
#include "eveio/Exception.hpp"

#include <cassert>
#include <cstddef>
#include <llhttp.h>

using namespace eveio;

namespace {

struct LLHttpParser {
  llhttp_t parser;
  void *ptr;
  const char *headerField;
  size_t headerFieldLength;
  const char *end;
};

static_assert(offsetof(LLHttpParser, parser) == 0,
              "Expect offset of parser to be 0");

eveio::HttpMethod ToHttpMethod(uint8_t i) noexcept {
  switch (i) {
  case HTTP_GET:
    return eveio::HttpMethod::Get;
  case HTTP_HEAD:
    return eveio::HttpMethod::Head;
  case HTTP_POST:
    return eveio::HttpMethod::Post;
  case HTTP_PUT:
    return eveio::HttpMethod::Put;
  case HTTP_DELETE:
    return eveio::HttpMethod::Delete;
  case HTTP_CONNECT:
    return eveio::HttpMethod::Connect;
  case HTTP_OPTIONS:
    return eveio::HttpMethod::Options;
  case HTTP_TRACE:
    return eveio::HttpMethod::Trace;
  case HTTP_PATCH:
    return eveio::HttpMethod::Patch;
  default:
    return eveio::HttpMethod::Undefined;
  }
}

} // namespace

eveio::HttpRequest::HttpRequest(AsyncTcpConnectionBuffer *data)
    : method(HttpMethod::Undefined), path(), headers(), body(), version{} {
  LLHttpParser parser;
  parser.ptr = this;
  parser.headerField = nullptr;
  parser.headerFieldLength = 0;
  parser.end = nullptr;

  llhttp_settings_t settings;
  llhttp_settings_init(&settings);
  settings.on_url = reinterpret_cast<llhttp_data_cb>(HttpRequest::OnURL);
  settings.on_header_field =
      reinterpret_cast<llhttp_data_cb>(HttpRequest::OnHeaderField);
  settings.on_header_value =
      reinterpret_cast<llhttp_data_cb>(HttpRequest::OnHeaderValue);
  settings.on_body = reinterpret_cast<llhttp_data_cb>(HttpRequest::OnBody);

  llhttp_init(&parser.parser, HTTP_REQUEST, &settings);
  int err = llhttp_execute(&parser.parser, data->data<char>(), data->size());

  if (err != HPE_OK) {
    data->clear();
    throw InvalidParameterException(
        __FILENAME__, __LINE__, __func__, "Invalid http request.");
  }

  err = llhttp_finish(&parser.parser);

  if (err != HPE_OK) {
    data->clear();
    throw InvalidParameterException(
        __FILENAME__, __LINE__, __func__, "Invalid http request.");
  }

  method = ToHttpMethod(parser.parser.method);
  version.major = parser.parser.http_major;
  version.minor = parser.parser.http_minor;

  assert(parser.end != nullptr);
  data->ReadOut(static_cast<const char *>(parser.end) - data->data<char>());
}

eveio::HttpRequest::~HttpRequest() noexcept = default;

int eveio::HttpRequest::OnURL(void *ptr, const char *at, size_t len) noexcept {
  auto parser = static_cast<LLHttpParser *>(ptr);
  auto self = static_cast<HttpRequest *>(parser->ptr);
  self->path = String(at, len);
  parser->end = at + len;
  return HPE_OK;
}

int eveio::HttpRequest::OnHeaderField(void *ptr,
                                      const char *at,
                                      size_t len) noexcept {
  auto parser = static_cast<LLHttpParser *>(ptr);
  parser->headerField = at;
  parser->headerFieldLength = len;
  parser->end = at + len;
  return HPE_OK;
}

int eveio::HttpRequest::OnHeaderValue(void *ptr,
                                      const char *at,
                                      size_t len) noexcept {
  auto parser = static_cast<LLHttpParser *>(ptr);
  auto self = static_cast<HttpRequest *>(parser->ptr);
  assert(parser->headerField != nullptr);
  self->headers.emplace(std::make_pair(
      String(parser->headerField, parser->headerFieldLength), String(at, len)));
  parser->headerField = nullptr;
  parser->headerFieldLength = 0;
  parser->end = at + len;
  return HPE_OK;
}

int eveio::HttpRequest::OnBody(void *ptr, const char *at, size_t len) noexcept {
  auto parser = static_cast<LLHttpParser *>(ptr);
  auto self = static_cast<HttpRequest *>(parser->ptr);
  self->body = String(at, len);
  parser->end = at + len;
  return HPE_OK;
}

eveio::HttpRequest::HttpRequest(const HttpRequest &) noexcept = default;
HttpRequest &

eveio::HttpRequest::operator=(const HttpRequest &) noexcept = default;

eveio::HttpRequest::HttpRequest(HttpRequest &&) noexcept = default;

HttpRequest &eveio::HttpRequest::operator=(HttpRequest &&) noexcept = default;

bool eveio::HttpRequest::ContainsHeader(const String &key) const noexcept {
  return headers.find(key) != headers.end();
}

const String *
eveio::HttpRequest::GetHeaderValue(const String &key) const noexcept {
  auto it = headers.find(key);
  if (it == headers.end()) {
    return nullptr;
  }
  return &it->second;
}

void eveio::HttpRequest::SetHeader(const String &key,
                                   const String &value) noexcept {
  headers[key] = value;
}

void eveio::HttpRequest::SetHeader(const String &key, String &&value) noexcept {
  headers[key] = std::move(value);
}

void eveio::HttpRequest::SetHeader(const String &key,
                                   StringRef value) noexcept {
  headers[key] = String(value.data(), value.size());
}

void eveio::HttpRequest::SetHeader(const String &key,
                                   const char *value) noexcept {
  headers[key] = String(value);
}
