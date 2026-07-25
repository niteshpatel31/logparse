#include "include/logger/formatter.hpp"
#include <chrono>
#include <ctime>
#include <fmt/format.h>

namespace logger {
Formatter::Formatter() : m_pattern("[%Y-%m-%d %H:%M:%S] [%L] [T%t] %v") {}

Formatter::Formatter(std::string pattern) : m_pattern(std::move(pattern)) {}

const std::string &Formatter::pattern() const noexcept { return m_pattern; }

void Formatter::format(fmt::memory_buffer &buffer,
                       const LogRecord &record) const {
  const auto time{std::chrono::system_clock::to_time_t(record.timestamp)};

#ifdef _WIN32
  std::tm tm{};
  localtime_s(&tm, &time);
#else
  std::tm tm{};
  localtime_r(&time, &tm);
#endif

  for (size_t i{0}; i < m_pattern.size(); ++i) {
    if (m_pattern[i] != '%') {
      buffer.push_back(m_pattern[i]);
      continue;
    }

    if (++i >= m_pattern.size())
      break;

    format_token(buffer, m_pattern[i], record);
  }
  buffer.push_back('\n');
}

void Formatter::format_token(fmt::memory_buffer &buffer, char token,
                             const LogRecord &record) const {
  const auto time{std::chrono::system_clock::to_time_t(record.timestamp)};
#ifdef _WIN32
  std::tm tm{};
  localtime_s(&tm, &time);
#else
  std::tm tm{};
  localtime_r(&time, &tm);
#endif
 switch (token)
    {
    case 'Y':
        fmt::format_to(
            std::back_inserter(buffer),
            "{:%Y}",
            tm);
        break;

    case 'm':
        fmt::format_to(
            std::back_inserter(buffer),
            "{:%m}",
            tm);
        break;

    case 'd':
        fmt::format_to(
            std::back_inserter(buffer),
            "{:%d}",
            tm);
        break;

    case 'H':
        fmt::format_to(
            std::back_inserter(buffer),
            "{:%H}",
            tm);
        break;

    case 'M':
        fmt::format_to(
            std::back_inserter(buffer),
            "{:%M}",
            tm);
        break;

    case 'S':
        fmt::format_to(
            std::back_inserter(buffer),
            "{:%S}",
            tm);
        break;

    case 'L':
        fmt::format_to(
            std::back_inserter(buffer),
            "{}",
            record.level);
        break;

    case 't':
        fmt::format_to(
            std::back_inserter(buffer),
            "{}",
            record.thread_id);
        break;

    case 'p':
        fmt::format_to(
            std::back_inserter(buffer),
            "{}",
            record.process_id);
        break;

    case 'f':
        fmt::format_to(
            std::back_inserter(buffer),
            "{}",
            record.file);
        break;

    case 'F':
        fmt::format_to(
            std::back_inserter(buffer),
            "{}",
            record.function);
        break;

    case 'l':
        fmt::format_to(
            std::back_inserter(buffer),
            "{}",
            record.line);
        break;

    case 'n':
        fmt::format_to(
            std::back_inserter(buffer),
            "{}",
            record.logger_name);
        break;

    case 'v':
        buffer.append(
            record.message.begin(),
            record.message.end());
        break;

    case '%':
        buffer.push_back('%');
        break;

    default:
        buffer.push_back('%');
        buffer.push_back(token);
        break;
    }
}

} // namespace logger
