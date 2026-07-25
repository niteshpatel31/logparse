#ifndef BACKEND_HPP
#define BACKEND_HPP
#include "formatter.hpp"
#include "record.hpp"
#include "sink.hpp"

namespace logger {

/**
 * @brief Abstract logging backend.
 *
 * A backend owns everything required to emit logs.
 *
 * Logger is only responsible for:
 *  - level filtering
 *  - constructing LogRecord
 *  - formatting user arguments
 *
 * After that the record is forwarded to the backend.
 *
 * This design completely removes
 *
 *      if(async)
 *
 * from the hot path.
 */
class Backend {
public:
  Backend(std::shared_ptr<Formatter> formatter,
          std::shared_ptr<SinkManager> sinks)
      : m_formatter(std::move(formatter)), m_sinks(std::move(sinks)) {}

  Backend(const Backend &) = delete;
  Backend &operator=(const Backend &) = delete;

  Backend(Backend &&) = delete;
  Backend &operator=(Backend &&) = delete;

  virtual ~Backend() = default;

  /**
   * Submit one log record.
   *
   * Thread-safe.
   */
  virtual void log(LogRecord &&record) = 0;

  /**
   * Flush every sink.
   *
   * Thread-safe.
   */
  virtual void flush() = 0;

protected:
  std::shared_ptr<Formatter> m_formatter;

  std::shared_ptr<SinkManager> m_sinks;
};

} // namespace logger

#endif
