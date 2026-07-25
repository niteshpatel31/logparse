#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>

#include "backend.hpp"
#include "config.hpp"
#include "queue.hpp"

namespace logger {

/**
 * @brief Asynchronous logging backend.
 *
 * Pipeline:
 *
 * Producer Thread
 *      │
 *      ▼
 *  Queue::push()
 *      │
 *      ▼
 * Worker Thread
 *      │
 *      ▼
 * Formatter
 *      │
 *      ▼
 * SinkManager
 *
 * Multiple producers.
 * Single consumer.
 */
class AsyncBackend final : public Backend {
public:
  AsyncBackend(std::shared_ptr<Formatter> formatter,
               std::shared_ptr<SinkManager> sinks,
               std::size_t queue_capacity = 65'536,
               OverflowPolicy policy = OverflowPolicy::Block);

  AsyncBackend(const AsyncBackend &) = delete;
  AsyncBackend &operator=(const AsyncBackend &) = delete;

  AsyncBackend(AsyncBackend &&) = delete;
  AsyncBackend &operator=(AsyncBackend &&) = delete;

  ~AsyncBackend() override;

  /**
   * Submit one log record.
   *
   * Thread-safe.
   */
  void log(LogRecord &&record) override;

  /**
   * Flush every pending message.
   *
   * Blocks until the worker has processed
   * everything currently in the queue.
   */
  void flush() override;

private:
  /**
   * Worker thread entry point.
   */
  void worker_loop();

  /**
   * Stop worker thread.
   */
  void stop();

private:
  Queue m_queue;

  std::thread m_worker;

  std::atomic<bool> m_running{true};

  std::mutex m_flush_mutex;

  std::condition_variable m_flush_cv;

  std::atomic<std::uint64_t> m_submitted{0};

  std::atomic<std::uint64_t> m_processed{0};
};

} // namespace logger
