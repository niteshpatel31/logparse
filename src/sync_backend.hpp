#include "include/logger/sync_backend.hpp"

#include <utility>

namespace logger {

namespace {

thread_local fmt::memory_buffer tls_buffer;

} // namespace

//=====================================================
// Constructor
//=====================================================

SyncBackend::SyncBackend(std::shared_ptr<Formatter> formatter,
                         std::shared_ptr<SinkManager> sinks)
    : Backend(std::move(formatter), std::move(sinks)) {}

//=====================================================
// Log
//=====================================================

void SyncBackend::log(LogRecord &&record) {
  tls_buffer.clear();

  m_formatter->format(tls_buffer, record);

  m_sinks->write(tls_buffer);
}

//=====================================================
// Flush
//=====================================================

void SyncBackend::flush() { m_sinks->flush(); }

} // namespace logger
