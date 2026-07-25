#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <climits>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

#include "level.hpp"

namespace logger {
/**
 * @brief Determines how the logger processes messages.
 */
enum class LoggerMode : bool { Sync, Async };

/**
 * @brief Queue overflow policy for asynchronous logging.
 */
enum class OverflowPolicy : std::uint8_t {
  Block,
  DropNewest,
  DropOldest,
  Discard
};

/**
 * @brief File rotation policy.
 */
struct RotationConfig {
  bool enabled{false};

  // rotate after this many bytes
  std::size_t max_file_size{10 * 1024 * 1024}; // 10MB

  // maximum number of rotated files to keep
  std::size_t max_file{5};
};

/**
 * @brief Runtime logger configuration.
 *
 * This structure is intentionally lightweight and owns all
 * configuration values. Logger instances receive a copy.
 */

struct Config {
  LoggerMode mode{LoggerMode::Async};
  Level level{Level::Info};
  bool colored_output{true};
  bool flush_on_error{true};

  std::string pattern{"[%Y-%m-%d %H:%M:%S] [%l] [T%t] %m"};

  // file sink
  bool enable_console{true};
  bool enable_file{false};
  std::filesystem::path file_path{"logs/application.log"};

  RotationConfig rotation{};

  // Async Logger
  std::size_t queue_capacity{UINT16_MAX + 1};

  OverflowPolicy overflowPolicy{OverflowPolicy::Block};

  // flush
  std::size_t flush_interval_ms{1000}; // 1 sec

  [[nodiscard]] bool async() const noexcept {
    return mode == LoggerMode::Async;
  }
  [[nodiscard]] bool sync() const noexcept { return mode == LoggerMode::Sync; }
};

} // namespace logger

#endif
