#ifndef EVEIO_RESULT_HPP
#define EVEIO_RESULT_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <variant>

namespace eveio {

template <class T, class Err = const char *>
class Result {
  std::variant<T, Err> value;
  bool is_valid;

  constexpr Result() noexcept = default;
  constexpr Result(T &&data) : value(std::forward<T>(data)), is_valid(true) {}
  constexpr Result(Err &&err)
      : value(std::forward<Err>(err)), is_valid(false) {}

public:
  ~Result() = default;

  template <class... Args,
            class = std::enable_if_t<
                std::is_constructible<std::decay_t<T>, Args...>::value,
                int>>
  static constexpr Result<T, Err> Ok(Args &&...args) {
    return Result<T, Err>{T{std::forward<Args>(args)...}};
  }

  static constexpr Result<T, Err> Ok(T &&data) {
    return Result<T, Err>{std::forward<T>(data)};
  }

  static constexpr Result<T, Err> Ok(T &data) {
    return Result<T, Err>{std::forward<T>(data)};
  }

  static constexpr Result<T, Err> Error(Err &&err) {
    return Result<T, Err>{std::forward<Err>(err)};
  }

  static constexpr Result<T, Err> Error(Err &err) {
    return Result<T, Err>{std::forward<Err>(err)};
  }

  constexpr bool IsValid() const noexcept { return is_valid; }

  constexpr const T &GetValue() const noexcept { return std::get<T>(value); }
  constexpr T &GetValue() noexcept { return std::get<T>(value); }

  constexpr const Err &GetError() const noexcept {
    return std::get<Err>(value);
  }
  constexpr Err &GetError() noexcept { return std::get<Err>(value); }
};

} // namespace eveio

#endif // EVEIO_RESULT_HPP
