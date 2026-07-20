#ifndef LEVEL_H
#define LEVEL_H

#include <array>
#include <cstdint>
#include <fmt/color.h>
#include <fmt/format.h>
#include <string_view>

namespace logger {

enum class Level : std::uint8_t {
  Trace = 0,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
  Off
};

namespace detail {
constexpr std::array<std::string_view, 7> KLevelNames{
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF"};

constexpr std::array<fmt::color, 7> KLevelColors{
    fmt::color::light_gray, fmt::color::cyan, fmt::color::green,
    fmt::color::yellow,     fmt::color::red,  fmt::color::crimson,
    fmt::color::white};
} // namespace detail

[[nodiscard]] constexpr std::string_view to_string(Level level) noexcept {
  return detail::KLevelNames[static_cast<std::size_t>(level)];
}

[[nodiscard]] constexpr fmt::color color(Level level) noexcept {
  return detail::KLevelColors[static_cast<std::size_t>(level)];
}

[[nodiscard]] constexpr bool operator<(Level lhs, Level rhs) noexcept {
  return static_cast<std::uint8_t>(lhs) < static_cast<std::uint8_t>(rhs);
}

[[nodiscard]] constexpr bool operator<=(Level lhs, Level rhs) noexcept {
  return static_cast<std::uint8_t>(lhs) <= static_cast<std::uint8_t>(rhs);
}

[[nodiscard]] constexpr bool operator>(Level lhs, Level rhs) noexcept {
  return static_cast<std::uint8_t>(lhs) > static_cast<std::uint8_t>(rhs);
}

[[nodiscard]] constexpr bool operator>=(Level lhs, Level rhs) noexcept {
  return static_cast<std::uint8_t>(lhs) >= static_cast<std::uint8_t>(rhs);
}

[[nodiscard]] constexpr bool operator==(Level lhs, Level rhs) noexcept {
  return static_cast<std::uint8_t>(lhs) == static_cast<std::uint8_t>(rhs);
}

[[nodiscard]] constexpr bool operator!=(Level lhs, Level rhs) noexcept {
  return static_cast<std::uint8_t>(lhs) != static_cast<std::uint8_t>(rhs);
}

} // namespace logger

/**
 * --------------------------------------------------------------------------
 * fmt formatter
 *
 * Allows:
 *
 * fmt::print("{}\n", logger::Level::Info);
 *
 * Output:
 *
 * INFO
 * --------------------------------------------------------------------------
 **/

template <>
struct fmt::formatter<logger::Level> : fmt::formatter<std::string_view> {

  template <typename FormatContext>
  auto format(logger::Level level, FormatContext &ctx) const {
    return fmt::formatter<std::string_view>::format(logger::to_string(level),
                                                    ctx);
  }
};

#endif
