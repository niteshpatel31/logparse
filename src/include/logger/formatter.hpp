#ifndef FORMATTER_HPP
#define FORMATTER_HPP

#include <chrono>
#include <fmt/chrono.h>
#include <fmt/format.h>

#include <string>
#include <string_view>

#include "record.hpp"

namespace logger {

/**
 * Supported formatter tokens.
 *
 * %Y  Year
 * %m  Month
 * %d  Day
 * %H  Hour
 * %M  Minute
 * %S  Second
 *
 * %L  Log Level
 * %t  Thread Id
 * %p  Process Id
 * %f  File
 * %F  Function
 * %l  Line
 * %n  Logger Name
 * %v  Message
 *
 * %%
 *      Literal %
 *
 * Example:
 *
 * [%Y-%m-%d %H:%M:%S] [%L] [%t] %v
 */

class Formatter {
public:
  Formatter();
  explicit Formatter(std::string patter);
  Formatter(const Formatter &) = default;
  Formatter &operator=(const Formatter &) = default;
  Formatter(Formatter &&) noexcept = default;
  Formatter &operator=(Formatter &&) noexcept = default;
  ~Formatter() = default;

  /**
   * Format a log record.
   *
   * Output is appended to buffer.
   * Buffer is NOT cleared.
   */
  void format(fmt::memory_buffer &buffer, const LogRecord &record) const;
  [[nodiscard]] const std::string &pattern() const noexcept;
  void set_pattern(const std::string &patter);

private:
  void format_token(fmt::memory_buffer &buffer, char token,
                    const LogRecord &record) const;
  std::string m_pattern;
};

} // namespace logger

#endif
