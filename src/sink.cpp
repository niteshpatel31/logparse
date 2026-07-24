#include "include/logger/sink.hpp"
#include <cstdio>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <ios>
#include <mutex>
#include <stdexcept>
#include <string_view>
namespace logger {
ConsoleSink::ConsoleSink(bool colored) : m_colored(colored) {}

void ConsoleSink::write(const fmt::memory_buffer &buffer) {
  std::lock_guard<std::mutex> lock{m_mutex};
  if (m_colored)
    fmt::print(fg(fmt::color::white), "{}",
               std::string_view(buffer.data(), buffer.size()));
  else
    fmt::print("{}", std::string_view(buffer.data(), buffer.size()));
  return;
}

void ConsoleSink::flush() {
  std::lock_guard<std::mutex> lock{m_mutex};
  std::fflush(stdout);
  return;
}

FileSink::FileSink(const std::filesystem::path &file) {
  std::filesystem::create_directories(file.parent_path());
  m_file.open(file, std::ios::app | std::ios::binary);
  if (!m_file)
    throw std::runtime_error("Failed to to open log file.");
  return;
}

void FileSink::write(const fmt::memory_buffer &buffer) {
  std::lock_guard<std::mutex> lock{m_mutex};
  m_file.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  return;
}

void FileSink::flush() {
  std::lock_guard<std::mutex> lock{m_mutex};
  m_file.flush();
  return;
}

bool FileSink::is_open() const noexcept { return m_file.is_open(); }

RotatingFileSink::RotatingFileSink(std::filesystem::path file,
                                   std::size_t max_file_size,
                                   std::size_t max_files)
    : m_file(std::move(file)), m_max_file_size(max_file_size),
      m_max_files(max_files) {
  std::filesystem::create_directories(m_file.parent_path());
  m_stream.open(m_file, std::ios::app | std::ios::binary);

  if (!m_stream)
    throw std::runtime_error("Failed to open log file.");
  return;
}

void RotatingFileSink::write(const fmt::memory_buffer &buffer) {
  std::lock_guard<std::mutex> lock{m_mutex};

  const size_t current_size{
      std::filesystem::exists(m_file) ? std::filesystem::file_size(m_file) : 0};

  if (current_size + buffer.size() >= m_max_file_size)
    rotate();
  m_stream.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  return;
}

void RotatingFileSink::flush() {
  std::lock_guard<std::mutex> lock{m_mutex};
  m_stream.flush();
  return;
}

void RotatingFileSink::rotate() {
  m_stream.close();
  for (std::size_t i{m_max_files}; i > 0; --i) {
    std::string src{m_file.string() + "." + std::to_string(i - 1)};
    std::string dest{m_file.string() + "." + std::to_string(i)};
    if (1 == i)
      src = m_file.string();
    if (std::filesystem::exists(src)) {
      std::error_code ec;
      std::filesystem::rename(src, dest, ec);
    }
  }
  m_stream.open(m_file, std::ios::app | std::ios::binary);
  if (!m_stream)
    throw std::runtime_error("Failed to reopen log file.");
  return;
}

void SinkManager::add_sink(std::unique_ptr<Sink> sink) {
  m_sinks.emplace_back(std::move(sink));
  return;
}

void SinkManager::write(const fmt::memory_buffer &buffer) {
  for (auto &sink : m_sinks)
    sink->write(buffer);
  return;
}

void SinkManager::flush() {
  for (auto &sink : m_sinks)
    sink->flush();
  return;
}

bool SinkManager::empty() const noexcept{return m_sinks.empty();}

std::size_t SinkManager::size()const noexcept{return m_sinks.size();}

} // namespace logger
