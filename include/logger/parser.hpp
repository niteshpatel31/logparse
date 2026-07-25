#ifndef LOGGER_PARSER_HPP
#define LOGGER_PARSER_HPP

#include "config.hpp"
#include "level.hpp"

#include <istream>
#include <optional>
#include <string>
#include <string_view>

namespace logger {

class ConfigParser {
public:
  [[nodiscard]] static Config parse(std::istream &input);
  [[nodiscard]] static Config parse_file(const std::string &path);

  [[nodiscard]] static std::optional<Level> parse_level(std::string_view value);
  [[nodiscard]] static std::optional<LoggerMode>
  parse_mode(std::string_view value);
  [[nodiscard]] static std::optional<OverflowPolicy>
  parse_overflow_policy(std::string_view value);
  [[nodiscard]] static std::optional<bool> parse_bool(std::string_view value);
};

} // namespace logger

#endif
