#include "logger/logger.hpp"

#include <chrono>
#include <thread>
#include <utility>

#ifdef _WIN32
#include <processthreadsapi.h>
#else
#include <unistd.h>
#endif

namespace logger {

namespace {

[[nodiscard]]
std::uint32_t process_id() noexcept {
#ifdef _WIN32
  return static_cast<std::uint32_t>(::GetCurrentProcessId());
#else
  return static_cast<std::uint32_t>(::getpid());
#endif
}

} // namespace

//=====================================================
// Constructor
//=====================================================

Logger::Logger(Config config)
    : m_config(std::move(config)), m_level(m_config.level),
      m_formatter(std::make_shared<Formatter>(m_config.pattern)),
      m_sinks(std::make_shared<SinkManager>()) {
  if (m_config.async()) {
    m_backend = std::make_unique<AsyncBackend>(
        m_formatter, m_sinks, m_config.queue_capacity, m_config.overflowPolicy);
  } else {
    m_backend = std::make_unique<SyncBackend>(m_formatter, m_sinks);
  }

  if (m_config.enable_console)
    add_sink<ConsoleSink>(m_config.colored_output);

  if (m_config.enable_file)
    add_sink<FileSink>(m_config.file_path);
}

//=====================================================
// Flush
//=====================================================

void Logger::flush() { m_backend->flush(); }

//=====================================================
// Level
//=====================================================

void Logger::set_level(Level level) noexcept {
  m_level.store(level, std::memory_order_relaxed);
}

Level Logger::level() const noexcept {
  return m_level.load(std::memory_order_relaxed);
}

//=====================================================
// Config
//=====================================================

const Config &Logger::config() const noexcept { return m_config; }

void Logger::set_pattern(std::string pattern) {
  m_config.pattern = std::move(pattern);
  m_formatter->set_pattern(m_config.pattern);
}

//=====================================================
// Record Creation
//=====================================================

LogRecord Logger::create_record(Level level,
                                const std::source_location &location) const {
  LogRecord record;

  record.timestamp = std::chrono::system_clock::now();

  record.level = level;

  record.thread_id = std::this_thread::get_id();

  record.process_id = process_id();

  record.file = location.file_name();

  record.function = location.function_name();

  record.line = location.line();

  return record;
}

} // namespace logger
