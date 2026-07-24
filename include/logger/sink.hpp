#ifndef SINK_HPP
#define SINK_HPP

#include <filesystem>
#include <fmt/color.h>
#include <fmt/format.h>

#include <fstream>
#include <memory>
#include <mutex>
#include <vector>

namespace logger {

/**
 * Base sink interface.
 *
 * Every sink receives an already formatted message.
 * Formatting must never happen inside a sink.
 */
class Sink {
public:
  Sink() = default;
  Sink(const Sink &) = delete;
  Sink &operator=(const Sink &) = delete;
  Sink(Sink &&) = default;
  Sink &operator=(Sink &&) = default;

  virtual ~Sink() = default;

  virtual void write(const fmt::memory_buffer &buffer) = 0;

  virtual void flush() = 0;
};

/**
 * Console sink.
 *
 * Thread-safe.
 */
class ConsoleSink final : public Sink {
public:
  explicit ConsoleSink(bool colored = true);

  void write(const fmt::memory_buffer &buffer) override;

  void flush() override;

private:
  bool m_colored;
  std::mutex m_mutex;
};

/**
 * File sink.
 *
 * Appends every log entry to a file.
 */
class FileSink final : public Sink {
public:
  explicit FileSink(const std::filesystem::path &file);

  void write(const fmt::memory_buffer &buffer) override;

  void flush() override;

  [[nodiscard]]
  bool is_open() const noexcept;

private:
  std::ofstream m_file;
  std::mutex m_mutex;
};

/**
 * Rotating file sink.
 *
 * Rotates when the file reaches max_file_size.
 */
class RotatingFileSink final : public Sink {
public:
  RotatingFileSink(std::filesystem::path file, std::size_t max_file_size,
                   std::size_t max_files);

  void write(const fmt::memory_buffer &buffer) override;

  void flush() override;

private:
  void rotate();

  std::filesystem::path m_file;

  std::ofstream m_stream;

  std::size_t m_max_file_size;
  std::size_t m_max_files;

  std::mutex m_mutex;
};

/**
 * Discards every message.
 *
 * Useful for benchmarks and testing.
 */
class NullSink final : public Sink {
public:
  void write(const fmt::memory_buffer &) override {}

  void flush() override {}
};

/**
 * Owns every sink used by a logger.
 *
 * Logger simply calls
 *
 * sinks.write(...)
 *
 * instead of iterating manually.
 */
class SinkManager {
public:
  SinkManager() = default;

  template <typename T, typename... Args> T &emplace_sink(Args &&...args) {
    auto sink = std::make_unique<T>(std::forward<Args>(args)...);

    T &ref = *sink;

    m_sinks.emplace_back(std::move(sink));

    return ref;
  }

  void add_sink(std::unique_ptr<Sink> sink);

  void write(const fmt::memory_buffer &buffer);

  void flush();

  [[nodiscard]]
  bool empty() const noexcept;

  [[nodiscard]]
  std::size_t size() const noexcept;

private:
  std::vector<std::unique_ptr<Sink>> m_sinks;
};

} // namespace logger

#endif
