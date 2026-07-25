#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <atomic>
#include <memory>
#include <source_location>
#include <string_view>
#include <utility>

#include <fmt/format.h>

#include "async_backend.hpp"
#include "backend.hpp"
#include "config.hpp"
#include "formatter.hpp"
#include "record.hpp"
#include "sink.hpp"
#include "sync_backend.hpp"

namespace logger {

/**
 * @brief Main logging interface.
 *
 * Logger is intentionally lightweight.
 *
 * Responsibilities:
 *  - Level filtering
 *  - Create LogRecord
 *  - Format user arguments
 *  - Forward record to backend
 *
 * Everything else is delegated to Backend.
 */
class Logger {
public:
  explicit Logger(Config config = {});

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  // Logger(Logger &&) noexcept = default;
  // Logger &operator=(Logger &&) noexcept = default;

  ~Logger() = default;

  //--------------------------------------------------
  // Logging
  //--------------------------------------------------

  template <typename... Args>
  void
  trace(fmt::format_string<Args...> fmt, Args &&...args,
        const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  debug(fmt::format_string<Args...> fmt, Args &&...args,
        const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  info(fmt::format_string<Args...> fmt, Args &&...args,
       const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  warn(fmt::format_string<Args...> fmt, Args &&...args,
       const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  error(fmt::format_string<Args...> fmt, Args &&...args,
        const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  fatal(fmt::format_string<Args...> fmt, Args &&...args,
        const std::source_location location = std::source_location::current());

  //--------------------------------------------------
  // Configuration
  //--------------------------------------------------

  void flush();

  void set_level(Level level) noexcept;

  [[nodiscard]]
  Level level() const noexcept;

  [[nodiscard]]
  const Config &config() const noexcept;

  void set_pattern(std::string pattern);

  //--------------------------------------------------
  // Sink Management
  //--------------------------------------------------

  template <typename SinkType, typename... Args>
  SinkType &add_sink(Args &&...args);

private:
  template <typename... Args>
  void log(Level level, const std::source_location &location,
           fmt::format_string<Args...> fmt, Args &&...args);

  [[nodiscard]]
  LogRecord create_record(Level level,
                          const std::source_location &location) const;

private:
  Config m_config;

  std::atomic<Level> m_level;

  std::shared_ptr<Formatter> m_formatter;

  std::shared_ptr<SinkManager> m_sinks;

  std::unique_ptr<Backend> m_backend;
};

//=====================================================
// Sink Management
//=====================================================

template <typename SinkType, typename... Args>
SinkType &Logger::add_sink(Args &&...args) {
  return m_sinks->emplace_sink<SinkType>(std::forward<Args>(args)...);
}

//=====================================================
// Logging API
//=====================================================

template <typename... Args>
void Logger::trace(fmt::format_string<Args...> fmt, Args &&...args,
                   const std::source_location location) {
  log(Level::Trace, location, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::debug(fmt::format_string<Args...> fmt, Args &&...args,
                   const std::source_location location) {
  log(Level::Debug, location, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::info(fmt::format_string<Args...> fmt, Args &&...args,
                  const std::source_location location) {
  log(Level::Info, location, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::warn(fmt::format_string<Args...> fmt, Args &&...args,
                  const std::source_location location) {
  log(Level::Warn, location, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::error(fmt::format_string<Args...> fmt, Args &&...args,
                   const std::source_location location) {
  log(Level::Error, location, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::fatal(fmt::format_string<Args...> fmt, Args &&...args,
                   const std::source_location location) {
  log(Level::Fatal, location, fmt, std::forward<Args>(args)...);
}

//=====================================================
// Core Logging
//=====================================================

template <typename... Args>
void Logger::log(Level level, const std::source_location &location,
                 fmt::format_string<Args...> fmt, Args &&...args) {
  if (level < m_level.load(std::memory_order_relaxed))
    return;

  LogRecord record = create_record(level, location);

  fmt::format_to(std::back_inserter(record.message), fmt,
                 std::forward<Args>(args)...);

  m_backend->log(std::move(record));
}

} // namespace logger

#endif
