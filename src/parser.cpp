#include "logger/parser.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <fstream>
#include <stdexcept>
#include <string>

namespace logger {

namespace {

[[nodiscard]] std::string trim(std::string_view value) {
  const auto first =
      std::find_if_not(value.begin(), value.end(),
                       [](unsigned char ch) { return std::isspace(ch) != 0; });

  const auto last =
      std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
      }).base();

  if (first >= last)
    return {};

  return std::string(first, last);
}

[[nodiscard]] std::string lower(std::string_view value) {
  std::string result{value};
  std::ranges::transform(result, result.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return result;
}

[[nodiscard]] std::optional<std::size_t> parse_size(std::string_view value) {
  std::size_t parsed{};
  const auto trimmed = trim(value);
  const auto *begin = trimmed.data();
  const auto *end = begin + trimmed.size();
  const auto [ptr, ec] = std::from_chars(begin, end, parsed);

  if (ec != std::errc{} || ptr != end)
    return std::nullopt;

  return parsed;
}

void apply(Config &config, std::string_view key, std::string_view value) {
  const auto normalized_key = lower(trim(key));
  const auto trimmed_value = trim(value);

  if (normalized_key == "mode") {
    if (auto mode = ConfigParser::parse_mode(trimmed_value))
      config.mode = *mode;
  } else if (normalized_key == "level") {
    if (auto level = ConfigParser::parse_level(trimmed_value))
      config.level = *level;
  } else if (normalized_key == "pattern") {
    config.pattern = trimmed_value;
  } else if (normalized_key == "queue_size" ||
             normalized_key == "queue_capacity") {
    if (auto size = parse_size(trimmed_value))
      config.queue_capacity = *size;
  } else if (normalized_key == "overflow_policy") {
    if (auto policy = ConfigParser::parse_overflow_policy(trimmed_value))
      config.overflowPolicy = *policy;
  } else if (normalized_key == "console" ||
             normalized_key == "console_enabled") {
    if (auto enabled = ConfigParser::parse_bool(trimmed_value))
      config.enable_console = *enabled;
  } else if (normalized_key == "file" || normalized_key == "file_enabled") {
    if (auto enabled = ConfigParser::parse_bool(trimmed_value))
      config.enable_file = *enabled;
  } else if (normalized_key == "file_path") {
    config.file_path = trimmed_value;
  } else if (normalized_key == "rotation" ||
             normalized_key == "rotation_enabled") {
    if (auto enabled = ConfigParser::parse_bool(trimmed_value))
      config.rotation.enabled = *enabled;
  } else if (normalized_key == "rotation_max_file_size") {
    if (auto size = parse_size(trimmed_value))
      config.rotation.max_file_size = *size;
  } else if (normalized_key == "rotation_max_files") {
    if (auto files = parse_size(trimmed_value))
      config.rotation.max_file = *files;
  } else if (normalized_key == "flush_interval" ||
             normalized_key == "flush_interval_ms") {
    if (auto interval = parse_size(trimmed_value))
      config.flush_interval_ms = *interval;
  } else if (normalized_key == "colored_output") {
    if (auto enabled = ConfigParser::parse_bool(trimmed_value))
      config.colored_output = *enabled;
  } else if (normalized_key == "flush_on_error") {
    if (auto enabled = ConfigParser::parse_bool(trimmed_value))
      config.flush_on_error = *enabled;
  }
}

} // namespace

Config ConfigParser::parse(std::istream &input) {
  Config config;
  std::string line;

  while (std::getline(input, line)) {
    const auto stripped = trim(line);

    if (stripped.empty() || stripped.front() == '#')
      continue;

    const auto delimiter = stripped.find('=');
    if (delimiter == std::string::npos)
      continue;

    apply(config, std::string_view{stripped}.substr(0, delimiter),
          std::string_view{stripped}.substr(delimiter + 1));
  }

  return config;
}

Config ConfigParser::parse_file(const std::string &path) {
  std::ifstream file{path};

  if (!file)
    throw std::runtime_error("failed to open logger config file: " + path);

  return parse(file);
}

std::optional<Level> ConfigParser::parse_level(std::string_view value) {
  const auto normalized = lower(trim(value));

  if (normalized == "trace")
    return Level::Trace;
  if (normalized == "debug")
    return Level::Debug;
  if (normalized == "info")
    return Level::Info;
  if (normalized == "warn" || normalized == "warning")
    return Level::Warn;
  if (normalized == "error")
    return Level::Error;
  if (normalized == "fatal")
    return Level::Fatal;
  if (normalized == "off")
    return Level::Off;

  return std::nullopt;
}

std::optional<LoggerMode> ConfigParser::parse_mode(std::string_view value) {
  const auto normalized = lower(trim(value));

  if (normalized == "sync" || normalized == "synchronous")
    return LoggerMode::Sync;
  if (normalized == "async" || normalized == "asynchronous")
    return LoggerMode::Async;

  return std::nullopt;
}

std::optional<OverflowPolicy>
ConfigParser::parse_overflow_policy(std::string_view value) {
  const auto normalized = lower(trim(value));

  if (normalized == "block")
    return OverflowPolicy::Block;
  if (normalized == "dropnewest" || normalized == "drop_newest")
    return OverflowPolicy::DropNewest;
  if (normalized == "dropoldest" || normalized == "drop_oldest")
    return OverflowPolicy::DropOldest;
  if (normalized == "discard")
    return OverflowPolicy::Discard;

  return std::nullopt;
}

std::optional<bool> ConfigParser::parse_bool(std::string_view value) {
  const auto normalized = lower(trim(value));

  if (normalized == "true" || normalized == "yes" || normalized == "on" ||
      normalized == "1")
    return true;
  if (normalized == "false" || normalized == "no" || normalized == "off" ||
      normalized == "0")
    return false;

  return std::nullopt;
}

} // namespace logger
