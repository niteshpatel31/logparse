#pragma once

#include "backend.hpp"

namespace logger {

/**
 * @brief Synchronous logging backend.
 *
 * Pipeline:
 *
 * LogRecord
 *      ↓
 * Formatter
 *      ↓
 * fmt::memory_buffer
 *      ↓
 * SinkManager
 */
class SyncBackend final : public Backend {
public:
  SyncBackend(std::shared_ptr<Formatter> formatter,
              std::shared_ptr<SinkManager> sinks);

  void log(LogRecord &&record) override;

  void flush() override;

private:
  /**
   * Scratch buffer reused for every write.
   *
   * Since synchronous logging performs formatting
   * on the caller thread, this buffer never crosses
   * thread boundaries.
   */
  fmt::memory_buffer m_buffer;
};

} // namespace logger
