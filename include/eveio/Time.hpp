#ifndef EVEIO_TIME_HPP
#define EVEIO_TIME_HPP

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <chrono>

namespace eveio {

class Time {
public:
  using Nanoseconds = std::chrono::nanoseconds;
  using Microseconds = std::chrono::microseconds;
  using Milliseconds = std::chrono::milliseconds;
  using Seconds = std::chrono::seconds;

  template <typename Duration>
  using TimePoint =
      std::chrono::time_point<std::chrono::system_clock, Duration>;

  template <typename To, typename From>
  constexpr static inline To duration_cast(const From &duration) noexcept {
    return std::chrono::duration_cast<To>(duration);
  }

  template <typename ToDur, typename From>
  constexpr static inline TimePoint<ToDur>
  time_point_cast(const From &time) noexcept {
    return std::chrono::time_point_cast<ToDur>(time);
  }

private:
  TimePoint<Nanoseconds> time_point;

public:
  Time() noexcept : time_point(Nanoseconds(0)) {}

  template <typename Duration>
  constexpr Time(TimePoint<Duration> time_point) noexcept
      : time_point(time_point_cast<Nanoseconds>(time_point)) {}

  explicit Time(std::uint64_t micro_secs_since_epoch) noexcept
      : time_point(Microseconds(micro_secs_since_epoch)) {}

  constexpr Time(const Time &other) noexcept : time_point(other.time_point) {}

  inline static Time Now() noexcept {
    return Time(std::chrono::system_clock::now());
  }

  template <typename Duration>
  constexpr inline Duration Diff(const Time &rhs) const noexcept {
    return duration_cast<Duration>(time_point - rhs.time_point);
  }

  template <typename Duration>
  constexpr inline TimePoint<Duration> ToChrono() const noexcept {
    return time_point_cast<Duration>(time_point);
  }

  inline Time &operator=(const Time &rhs) noexcept {
    time_point = rhs.time_point;
    return (*this);
  }

  constexpr inline bool operator==(const Time &rhs) const noexcept {
    return time_point == rhs.time_point;
  }

  constexpr inline bool operator!=(const Time &rhs) const noexcept {
    return time_point != rhs.time_point;
  }

  constexpr inline bool operator<(const Time &rhs) const noexcept {
    return time_point < rhs.time_point;
  }

  constexpr inline bool operator<=(const Time &rhs) const noexcept {
    return time_point <= rhs.time_point;
  }

  constexpr inline bool operator>(const Time &rhs) const noexcept {
    return time_point > rhs.time_point;
  }

  constexpr inline bool operator>=(const Time &rhs) const noexcept {
    return time_point >= rhs.time_point;
  }

  template <typename Duration>
  inline Time operator+(const Duration &rhs) const noexcept {
    return time_point + rhs;
  }

  template <typename Duration>
  inline Time operator-(const Duration &rhs) const noexcept {
    return time_point - rhs;
  }

  template <class, class, class>
  friend struct fmt::formatter;
};

} // namespace eveio

template <typename CharT>
struct fmt::formatter<eveio::Time, CharT>
    : fmt::formatter<std::chrono::system_clock::time_point, CharT> {
  template <typename FormatContext>
  auto format(const eveio::Time &t, FormatContext &ctx) -> decltype(ctx.out()) {
    return fmt::formatter<std::chrono::system_clock::time_point, CharT>::format(
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            t.time_point),
        ctx);
  }
};

#endif // EVEIO_TIME_HPP
