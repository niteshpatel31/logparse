#ifndef RECORD_HPP
#define RECORD_HPP

#include "level.hpp"
#include <chrono>
#include <fmt/format.h>
#include <source_location>
#include <thread>

namespace logger {
/**
 * @brief Represents a single log event.
 *
 * A LogRecord is created by the Logger, formatted by the Formatter, and
 * consumed by one or more Sink implementations.
 *
 * Ownership: Producer Thread ->
 * Queue -> Worker Thread -> Sink */

struct LogRecord {
  // Timestamp captured when the record is created.
  std::chrono::system_clock::time_point timestamp;

  // Severity level.
  Level level{Level::Info};

  // Thread that produced the log.
  std::thread::id thread_id;

  // Process identifier.
  std::uint32_t process_id{0u};

  // Source file.
  std::string_view file;

  // Function name.
  std::string_view function;

  // Source line.
  std::uint_least32_t line{0u};

  /**
   * Formatted message.
   *
   * Using fmt::memory_buffer avoids an additional heap allocation
   * for most log messages (inline storage).
   */
  fmt::memory_buffer message;

  /**
   * Optional logger/module name. *
   * Example:
   *    "network"
   *    "database"
   *    "renderer"
   */
  std::string_view logger_name{};

  /**
   * Capture current source location.
   *
   * Default arguments allow callers to simply write:
   *
   * LogRecord record{ .level = Level::Info,... };
   */

  static constexpr std::source_location
  current_location(const std::source_location location =
                       std::source_location::current()) noexcept {
    return location;
  }

  [[nodiscard]]
  constexpr std::size_t message_size() const noexcept {
    return message.size();
  }

  [[nodiscard]]
  constexpr bool empty() const noexcept {
    return message.size() == 0; // no message.empty();
  }

  void clear() noexcept { message.clear(); }
};

} // namespace logger

#endif
