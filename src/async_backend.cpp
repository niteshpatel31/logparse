#include "include/logger/async_backend.hpp"

#include <utility>

namespace logger {

namespace {

thread_local fmt::memory_buffer tls_buffer;

} // namespace

//=====================================================
// Constructor
//=====================================================

AsyncBackend::AsyncBackend(std::shared_ptr<Formatter> formatter,
                           std::shared_ptr<SinkManager> sinks,
                           std::size_t queue_capacity, OverflowPolicy policy)
    : Backend(std::move(formatter), std::move(sinks)),
      m_queue(queue_capacity, policy),
      m_worker(&AsyncBackend::worker_loop, this) {}

//=====================================================
// Destructor
//=====================================================

AsyncBackend::~AsyncBackend() { stop(); }

//=====================================================
// Submit Log
//=====================================================

void AsyncBackend::log(LogRecord &&record) {
  if (m_queue.push(std::move(record))) {
    m_submitted.fetch_add(1, std::memory_order_relaxed);
  }
}

//=====================================================
// Flush
//=====================================================

void AsyncBackend::flush() {
  std::unique_lock lock(m_flush_mutex);

  m_flush_cv.wait(lock, [this] {
    return m_processed.load(std::memory_order_acquire) ==
           m_submitted.load(std::memory_order_acquire);
  });

  m_sinks->flush();
}

//=====================================================
// Stop
//=====================================================

void AsyncBackend::stop() {
  bool expected = true;

  if (!m_running.compare_exchange_strong(expected, false)) {
    return;
  }

  m_queue.shutdown();

  if (m_worker.joinable())
    m_worker.join();
}

//=====================================================
// Worker
//=====================================================

void AsyncBackend::worker_loop() {
  LogRecord record;

  while (m_running.load(std::memory_order_acquire)) {
    if (!m_queue.pop(record))
      continue;

    tls_buffer.clear();

    try {
      m_formatter->format(tls_buffer, record);

      m_sinks->write(tls_buffer);
    } catch (...) {
      // Logging must never throw.
    }

    const auto processed =
        m_processed.fetch_add(1, std::memory_order_release) + 1;

    if (processed == m_submitted.load(std::memory_order_acquire)) {
      std::lock_guard lock(m_flush_mutex);

      m_flush_cv.notify_all();
    }
  }

  //
  // Drain remaining messages
  //
  while (m_queue.pop(record)) {
    tls_buffer.clear();

    try {
      m_formatter->format(tls_buffer, record);

      m_sinks->write(tls_buffer);
    } catch (...) {
    }

    m_processed.fetch_add(1, std::memory_order_release);
  }

  m_sinks->flush();
}

} // namespace logger
