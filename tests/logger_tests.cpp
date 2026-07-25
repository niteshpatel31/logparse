#include "logger/formatter.hpp"
#include "logger/parser.hpp"

#include <chrono>
#include <sstream>

#include <gtest/gtest.h>

TEST(Formatter, FormatsCompiledDefaultTokens) {
  logger::Formatter formatter{"[%Y-%m-%d %H:%M:%S] [%l] %m"};
  logger::LogRecord record;
  fmt::memory_buffer output;

  record.timestamp = std::chrono::system_clock::from_time_t(0);
  record.level = logger::Level::Info;
  fmt::format_to(std::back_inserter(record.message), "ready");

  formatter.format(output, record);

  const std::string_view formatted{output.data(), output.size()};
  EXPECT_NE(formatted.find("[INFO] ready"), std::string_view::npos);
}

TEST(ConfigParser, ParsesKeyValueConfig) {
  std::istringstream input{"mode=sync\n"
                           "level=debug\n"
                           "queue_capacity=1024\n"
                           "console=false\n"
                           "overflow_policy=drop_oldest\n"};

  const auto config = logger::ConfigParser::parse(input);

  EXPECT_EQ(config.mode, logger::LoggerMode::Sync);
  EXPECT_EQ(config.level, logger::Level::Debug);
  EXPECT_EQ(config.queue_capacity, 1024U);
  EXPECT_FALSE(config.enable_console);
  EXPECT_EQ(config.overflowPolicy, logger::OverflowPolicy::DropOldest);
}
