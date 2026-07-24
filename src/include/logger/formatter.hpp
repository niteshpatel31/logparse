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
  explicit Formatter(std::string& patter);
};

} // namespace logger

#endif
