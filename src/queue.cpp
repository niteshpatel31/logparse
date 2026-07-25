#include "logger/queue.hpp"

#include <utility>

namespace logger {
Queue::Queue(size_t capacity, OverflowPolicy policy)
    : m_buffer(capacity), m_capacity{capacity}, m_policy{policy} {}

bool Queue::push(LogRecord &&record) {
  std::unique_lock lock(m_mutex);

  switch (m_policy) {
  case OverflowPolicy::Block:

    m_not_full.wait(lock, [this] { return m_size < m_capacity || m_shutdown; });

    if (m_shutdown)
      return false;

    break;

  case OverflowPolicy::DropNewest:

    if (m_size == m_capacity)
      return false;

    break;

  case OverflowPolicy::DropOldest:

    if (m_size == m_capacity) {
      m_head = (m_head + 1) % m_capacity;
      --m_size;
    }

    break;

  case OverflowPolicy::Discard:

    if (m_size == m_capacity)
      return false;

    break;
  }

  m_buffer[m_tail] = std::move(record);

  m_tail = (m_tail + 1) % m_capacity;

  ++m_size;

  lock.unlock();

  m_not_empty.notify_one();

  return true;
}

//=====================================================
// Pop
//=====================================================

bool Queue::pop(LogRecord &record) {
  std::unique_lock lock(m_mutex);

  m_not_empty.wait(lock, [this] { return m_size > 0 || m_shutdown; });

  if (m_size == 0)
    return false;

  record = std::move(m_buffer[m_head]);

  m_head = (m_head + 1) % m_capacity;

  --m_size;

  lock.unlock();

  m_not_full.notify_one();

  return true;
}

//=====================================================
// Shutdown
//=====================================================

void Queue::shutdown() {
  {
    std::lock_guard lock(m_mutex);

    m_shutdown = true;
  }

  m_not_empty.notify_all();

  m_not_full.notify_all();
}

//=====================================================
// State
//=====================================================

bool Queue::empty() const noexcept {
  std::lock_guard lock(m_mutex);

  return m_size == 0;
}

bool Queue::full() const noexcept {
  std::lock_guard lock(m_mutex);

  return m_size == m_capacity;
}

std::size_t Queue::size() const noexcept {
  std::lock_guard lock(m_mutex);

  return m_size;
}

std::size_t Queue::capacity() const noexcept { return m_capacity; }
} // namespace logger
