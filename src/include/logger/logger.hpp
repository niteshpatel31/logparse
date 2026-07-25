#pragma once

#include <atomic>
#include <memory>
#include <source_location>
#include <string_view>

#include <fmt/format.h>

#include "config.hpp"
#include "formatter.hpp"
#include "queue.hpp"
#include "sink.hpp"

namespace logger {

class Logger {
public:
  explicit Logger(Config config = {});

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  Logger(Logger &&) = delete;
  Logger &operator=(Logger &&) = delete;

  ~Logger();

  //--------------------------------------------------
  // Logging API
  //--------------------------------------------------

  template <typename... Args>
  void
  trace(fmt::format_string<Args...> format, Args &&...args,
        const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  debug(fmt::format_string<Args...> format, Args &&...args,
        const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  info(fmt::format_string<Args...> format, Args &&...args,
       const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  warn(fmt::format_string<Args...> format, Args &&...args,
       const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  error(fmt::format_string<Args...> format, Args &&...args,
        const std::source_location location = std::source_location::current());

  template <typename... Args>
  void
  fatal(fmt::format_string<Args...> format, Args &&...args,
        const std::source_location location = std::source_location::current());

  //--------------------------------------------------
  // Sink Management
  //--------------------------------------------------

  template <typename SinkType, typename... Args>
  SinkType &add_sink(Args &&...args);

  void flush();

  //--------------------------------------------------
  // Configuration
  //--------------------------------------------------

  void set_level(Level level) noexcept;

  [[nodiscard]]
  Level level() const noexcept;

  void set_pattern(std::string pattern);

  [[nodiscard]]
  const Config &config() const noexcept;

private:
  template <typename... Args>
  void log(Level level, const std::source_location &location,
           fmt::format_string<Args...> format, Args &&...args);

  [[nodiscard]]
  LogRecord create_record(Level level,
                          const std::source_location &location) const;

private:
  Config m_config;

  Formatter m_formatter;

  SinkManager m_sinks;

  Queue m_queue;

  std::atomic<Level> m_level;
};

//======================================================
// Template Implementation
//======================================================

template <typename SinkType, typename... Args>
SinkType &Logger::add_sink(Args &&...args) {
  return m_sinks.emplace_sink<SinkType>(std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::trace(fmt::format_string<Args...> format, Args &&...args,
                   const std::source_location location) {
  log(Level::Trace, location, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::debug(fmt::format_string<Args...> format, Args &&...args,
                   const std::source_location location) {
  log(Level::Debug, location, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::info(fmt::format_string<Args...> format, Args &&...args,
                  const std::source_location location) {
  log(Level::Info, location, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::warn(fmt::format_string<Args...> format, Args &&...args,
                  const std::source_location location) {
  log(Level::Warn, location, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::error(fmt::format_string<Args...> format, Args &&...args,
                   const std::source_location location) {
  log(Level::Error, location, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::fatal(fmt::format_string<Args...> format, Args &&...args,
                   const std::source_location location) {
  log(Level::Fatal, location, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Logger::log(Level level, const std::source_location &location,
                 fmt::format_string<Args...> format, Args &&...args) {
  if (level < m_level.load(std::memory_order_relaxed))
    return;

  LogRecord record = create_record(level, location);

  fmt::format_to(std::back_inserter(record.message), format,
                 std::forward<Args>(args)...);

  if (m_config.async()) {
    m_queue.push(std::move(record));
    return;
  }

  fmt::memory_buffer buffer;

  m_formatter.format(buffer, record);

  m_sinks.write(buffer);

  if (m_config.flush_on_error && level >= Level::Error) {
    m_sinks.flush();
  }
}

} // namespace logger
