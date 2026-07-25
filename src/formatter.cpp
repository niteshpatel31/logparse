#include "logger/formatter.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <functional>
#include <iterator>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <utility>

namespace logger {

namespace {

constexpr std::string_view kDefaultPattern{"[%Y-%m-%d %H:%M:%S] [%l] [T%t] %m"};
constexpr std::string_view kTimestampPattern{"%Y-%m-%d %H:%M:%S"};

[[nodiscard]] std::tm local_time(std::time_t time) noexcept {
  std::tm tm{};

#ifdef _WIN32
  localtime_s(&tm, &time);
#else
  localtime_r(&time, &tm);
#endif

  return tm;
}

[[nodiscard]] bool starts_with(std::string_view value, std::size_t offset,
                               std::string_view prefix) noexcept {
  return offset + prefix.size() <= value.size() &&
         value.substr(offset, prefix.size()) == prefix;
}

} // namespace

Formatter::Formatter() : Formatter(std::string{kDefaultPattern}) {}

Formatter::Formatter(std::string pattern)
    : m_pattern(std::move(pattern)), m_tokens(compile_pattern(m_pattern)) {}

std::string Formatter::pattern() const {
  std::shared_lock lock{m_mutex};
  return m_pattern;
}

void Formatter::set_pattern(std::string pattern) {
  auto tokens = compile_pattern(pattern);

  std::unique_lock lock{m_mutex};
  m_pattern = std::move(pattern);
  m_tokens = std::move(tokens);
}

void Formatter::format(fmt::memory_buffer &buffer,
                       const LogRecord &record) const {
  std::shared_lock lock{m_mutex};

  for (const auto &token : m_tokens)
    append_token(buffer, token, record);

  buffer.push_back('\n');
}

std::vector<Formatter::Token>
Formatter::compile_pattern(std::string_view pattern) {
  std::vector<Token> tokens;
  std::string literal;

  const auto flush_literal = [&] {
    if (!literal.empty()) {
      tokens.push_back(Token{TokenKind::Literal, std::move(literal)});
      literal.clear();
    }
  };

  for (std::size_t i = 0; i < pattern.size(); ++i) {
    if (starts_with(pattern, i, kTimestampPattern)) {
      flush_literal();
      tokens.push_back(Token{TokenKind::Timestamp, {}});
      i += kTimestampPattern.size() - 1;
      continue;
    }

    if (pattern[i] != '%') {
      literal.push_back(pattern[i]);
      continue;
    }

    if (++i >= pattern.size()) {
      literal.push_back('%');
      break;
    }

    flush_literal();

    switch (pattern[i]) {
    case 'Y':
      tokens.push_back(Token{TokenKind::Year, {}});
      break;
    case 'd':
      tokens.push_back(Token{TokenKind::Day, {}});
      break;
    case 'H':
      tokens.push_back(Token{TokenKind::Hour, {}});
      break;
    case 'M':
      tokens.push_back(Token{TokenKind::Minute, {}});
      break;
    case 'S':
      tokens.push_back(Token{TokenKind::Second, {}});
      break;
    case 'l':
    case 'L':
      tokens.push_back(Token{TokenKind::Level, {}});
      break;
    case 't':
      tokens.push_back(Token{TokenKind::ThreadId, {}});
      break;
    case 'p':
      tokens.push_back(Token{TokenKind::ProcessId, {}});
      break;
    case 'f':
      tokens.push_back(Token{TokenKind::File, {}});
      break;
    case 'F':
      tokens.push_back(Token{TokenKind::Function, {}});
      break;
    case '#':
      tokens.push_back(Token{TokenKind::Line, {}});
      break;
    case 'n':
      tokens.push_back(Token{TokenKind::LoggerName, {}});
      break;
    case 'm':
    case 'v':
      tokens.push_back(Token{TokenKind::Message, {}});
      break;
    case '%':
      tokens.push_back(Token{TokenKind::Literal, "%"});
      break;
    default:
      literal.push_back('%');
      literal.push_back(pattern[i]);
      break;
    }
  }

  flush_literal();
  tokens.shrink_to_fit();
  return tokens;
}

void Formatter::append_token(fmt::memory_buffer &buffer, const Token &token,
                             const LogRecord &record) {
  switch (token.kind) {
  case TokenKind::Literal:
    buffer.append(token.literal.data(),
                  token.literal.data() + token.literal.size());
    break;
  case TokenKind::Timestamp:
    append_timestamp(buffer, record);
    break;
  case TokenKind::Year:
  case TokenKind::Month:
  case TokenKind::Day:
  case TokenKind::Hour:
  case TokenKind::Minute:
  case TokenKind::Second:
    append_time_component(buffer, token.kind, record);
    break;
  case TokenKind::Level:
    fmt::format_to(std::back_inserter(buffer), "{}", record.level);
    break;
  case TokenKind::ThreadId:
    fmt::format_to(std::back_inserter(buffer), "{}",
                   std::hash<std::thread::id>{}(record.thread_id));
    break;
  case TokenKind::ProcessId:
    fmt::format_to(std::back_inserter(buffer), "{}", record.process_id);
    break;
  case TokenKind::File:
    fmt::format_to(std::back_inserter(buffer), "{}", record.file);
    break;
  case TokenKind::Function:
    fmt::format_to(std::back_inserter(buffer), "{}", record.function);
    break;
  case TokenKind::Line:
    fmt::format_to(std::back_inserter(buffer), "{}", record.line);
    break;
  case TokenKind::LoggerName:
    fmt::format_to(std::back_inserter(buffer), "{}", record.logger_name);
    break;
  case TokenKind::Message:
    buffer.append(record.message.begin(), record.message.end());
    break;
  }
}

void Formatter::append_timestamp(fmt::memory_buffer &buffer,
                                 const LogRecord &record) {
  const auto seconds = std::chrono::system_clock::to_time_t(record.timestamp);

  thread_local std::time_t cached_seconds{};
  thread_local bool cache_valid{false};
  thread_local fmt::memory_buffer cached_timestamp;

  if (!cache_valid || cached_seconds != seconds) {
    cached_seconds = seconds;
    cache_valid = true;
    cached_timestamp.clear();

    const auto tm = local_time(seconds);
    fmt::format_to(std::back_inserter(cached_timestamp), "{:%Y-%m-%d %H:%M:%S}",
                   tm);
  }

  buffer.append(cached_timestamp.data(),
                cached_timestamp.data() + cached_timestamp.size());
}

void Formatter::append_time_component(fmt::memory_buffer &buffer,
                                      TokenKind kind, const LogRecord &record) {
  const auto seconds = std::chrono::system_clock::to_time_t(record.timestamp);
  const auto tm = local_time(seconds);

  switch (kind) {
  case TokenKind::Year:
    fmt::format_to(std::back_inserter(buffer), "{:%Y}", tm);
    break;
  case TokenKind::Month:
    fmt::format_to(std::back_inserter(buffer), "{:%m}", tm);
    break;
  case TokenKind::Day:
    fmt::format_to(std::back_inserter(buffer), "{:%d}", tm);
    break;
  case TokenKind::Hour:
    fmt::format_to(std::back_inserter(buffer), "{:%H}", tm);
    break;
  case TokenKind::Minute:
    fmt::format_to(std::back_inserter(buffer), "{:%M}", tm);
    break;
  case TokenKind::Second:
    fmt::format_to(std::back_inserter(buffer), "{:%S}", tm);
    break;
  default:
    break;
  }
}

} // namespace logger
