#ifndef FORMATTER_HPP
#define FORMATTER_HPP

#include <chrono>
#include <fmt/chrono.h>
#include <fmt/format.h>

#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

#include "record.hpp"

namespace logger {

/**
 * Supported formatter tokens.
 *
 * %Y  Year
 * %m  Month when used inside %Y-%m-%d %H:%M:%S
 * %d  Day
 * %H  Hour
 * %M  Minute
 * %S  Second
 *
 * %l  Log Level
 * %L  Log Level (compatibility alias)
 * %t  Thread Id
 * %p  Process Id
 * %f  File
 * %F  Function
 * %#  Line
 * %n  Logger Name
 * %m  Message
 * %v  Message (compatibility alias)
 *
 * %%
 *      Literal %
 *
 * Example:
 *
 * [%Y-%m-%d %H:%M:%S] [%l] [%t] %m
 */

class Formatter {
public:
  Formatter();
  explicit Formatter(std::string pattern);
  Formatter(const Formatter &) = delete;
  Formatter &operator=(const Formatter &) = delete;
  Formatter(Formatter &&) noexcept = delete;
  Formatter &operator=(Formatter &&) noexcept = delete;
  ~Formatter() = default;

  /**
   * Format a log record.
   *
   * Output is appended to buffer.
   * Buffer is NOT cleared.
   */
  void format(fmt::memory_buffer &buffer, const LogRecord &record) const;
  [[nodiscard]] std::string pattern() const;
  void set_pattern(std::string pattern);

private:
  enum class TokenKind {
    Literal,
    Timestamp,
    Year,
    Month,
    Day,
    Hour,
    Minute,
    Second,
    Level,
    ThreadId,
    ProcessId,
    File,
    Function,
    Line,
    LoggerName,
    Message
  };

  struct Token {
    TokenKind kind;
    std::string literal;
  };

  [[nodiscard]] static std::vector<Token>
  compile_pattern(std::string_view pattern);

  static void append_token(fmt::memory_buffer &buffer, const Token &token,
                           const LogRecord &record);
  static void append_timestamp(fmt::memory_buffer &buffer,
                               const LogRecord &record);
  static void append_time_component(fmt::memory_buffer &buffer, TokenKind kind,
                                    const LogRecord &record);

  std::string m_pattern;
  std::vector<Token> m_tokens;
  mutable std::shared_mutex m_mutex;
};

} // namespace logger

#endif
