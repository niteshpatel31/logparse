#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <vector>

#include "config.hpp"
#include "record.hpp"

namespace logger {
/**
 * Bounded MPSC queue.
 *
 * Multiple producer threads.
 * Single consumer thread.
 *
 * Memory is allocated once during construction.
 */

class Queue {
public:
  explicit Queue(size_t capacity,
                 OverflowPolicy policy = OverflowPolicy::Block);
  Queue(const Queue &) = delete;
  Queue &operator=(const Queue &) = delete;
  Queue(Queue &&) = delete;
  Queue &operator=(Queue &&) = delete;

  ~Queue() = default;

  /**
   * Push a record.
   *
   * Returns false if the configured overflow
   * policy drops the message.
   */
  [[nodiscard]]
  bool push(LogRecord &&record);

  /**
   * Pop one record.
   *
   * Blocks until either:
   *
   *  - an item exists
   *  - shutdown() is called
   */
  [[nodiscard]]
  bool pop(LogRecord &record);

  /**
   * Wake every waiting thread.
   */
  void shutdown();

  [[nodiscard]]
  bool empty() const noexcept;

  [[nodiscard]]
  bool full() const noexcept;

  [[nodiscard]]
  std::size_t size() const noexcept;

  [[nodiscard]]
  std::size_t capacity() const noexcept;

private:
  std::vector<LogRecord> m_buffer;

  std::size_t m_head{0};

  std::size_t m_tail{0};

  std::size_t m_size{0};

  std::size_t m_capacity;

  OverflowPolicy m_policy;

  bool m_shutdown{false};

  mutable std::mutex m_mutex;

  std::condition_variable m_not_empty;

  std::condition_variable m_not_full;
};
} // namespace logger

#endif
