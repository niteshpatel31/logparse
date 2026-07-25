#include "logger/sync_backend.hpp"

#include <utility>

namespace logger {

namespace {

thread_local fmt::memory_buffer tls_buffer;

} // namespace

SyncBackend::SyncBackend(std::shared_ptr<Formatter> formatter,
                         std::shared_ptr<SinkManager> sinks)
    : Backend(std::move(formatter), std::move(sinks)) {}

void SyncBackend::log(LogRecord &&record) {
  tls_buffer.clear();

  m_formatter->format(tls_buffer, record);

  m_sinks->write(tls_buffer);
}

void SyncBackend::flush() { m_sinks->flush(); }

} // namespace logger
