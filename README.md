# logparse

`logparse` is a small C++20 logging library. It can write logs to the console or to files, run in sync or async mode, and format each log line with a simple pattern.

It uses CMake for builds and `fmt` for message formatting.

## What It Does

- Logs messages with levels like `info`, `warn`, and `error`
- Supports sync and async logging
- Writes to the console, files, rotating files, or a null sink
- Uses `fmt` style messages, like `"user {} logged in"`
- Can read logger settings from a simple `key=value` config file
- Includes an example app, tests, and an optional benchmark

## Requirements

You need:

- A C++20 compiler
- CMake 3.24 or newer
- `fmt`

Optional tools:

- GTest, if you want to run tests
- Google Benchmark, if you want to run benchmarks

On Debian or Ubuntu:

```sh
sudo apt install cmake g++ libfmt-dev libgtest-dev libbenchmark-dev
```

On macOS with Homebrew:

```sh
brew install cmake fmt googletest google-benchmark
```

## Build

From the project root:

```sh
cmake -S . -B build
cmake --build build
```

Run the basic example:

```sh
./build/logger_basic_example
```

Run the tests:

```sh
ctest --test-dir build --output-on-failure
```

Build and run the benchmark:

```sh
cmake -S . -B build -DLOGGER_BUILD_BENCHMARKS=ON
cmake --build build
./build/logger_benchmark
```

## Build Options

| Option | Default | What it does |
| --- | --- | --- |
| `LOGGER_BUILD_EXAMPLES` | `ON` | Builds the basic example program. |
| `LOGGER_BUILD_TESTS` | `ON` | Builds tests when GTest is installed. |
| `LOGGER_BUILD_BENCHMARKS` | `OFF` | Builds benchmarks when Google Benchmark is installed. |

## Quick Example

```cpp
#include "logger/logger.hpp"

int main() {
  logger::Config config;
  config.mode = logger::LoggerMode::Sync;
  config.level = logger::Level::Info;
  config.pattern = "[%Y-%m-%d %H:%M:%S] [%l] %m";

  logger::Logger log{config};

  log.info("logger initialized");
  log.warn("cache miss for key {}", "user:42");
  log.error("request failed with status {}", 500);

  log.flush();
}
```

Available log calls:

```cpp
log.trace("message {}", value);
log.debug("message {}", value);
log.info("message {}", value);
log.warn("message {}", value);
log.error("message {}", value);
log.fatal("message {}", value);
```

The logger ignores messages below the current level.

## Logger Config

Most behavior is set through `logger::Config`.

| Field | Default | Meaning |
| --- | --- | --- |
| `mode` | `LoggerMode::Async` | Use async or sync logging. |
| `level` | `Level::Info` | Lowest level that will be logged. |
| `colored_output` | `true` | Use colored console output. |
| `flush_on_error` | `true` | Config value for flushing on errors. |
| `pattern` | `[%Y-%m-%d %H:%M:%S] [%l] [T%t] %m` | Format for each log line. |
| `enable_console` | `true` | Add a console sink automatically. |
| `enable_file` | `false` | Add a file sink automatically. |
| `file_path` | `logs/application.log` | File used by the default file sink. |
| `rotation.enabled` | `false` | Rotation setting. |
| `rotation.max_file_size` | `10485760` | Rotate after this many bytes. |
| `rotation.max_file` | `5` | Number of rotated files to keep. |
| `queue_capacity` | `65536` | Size of the async queue. |
| `overflowPolicy` | `OverflowPolicy::Block` | What async mode does when the queue is full. |
| `flush_interval_ms` | `1000` | Flush interval setting in milliseconds. |

Example:

```cpp
logger::Config config;
config.mode = logger::LoggerMode::Async;
config.level = logger::Level::Debug;
config.enable_console = true;
config.enable_file = true;
config.file_path = "logs/app.log";
config.queue_capacity = 8192;
config.overflowPolicy = logger::OverflowPolicy::DropOldest;

logger::Logger log{config};
```

## Config Files

You can also load settings from a plain text file.

```ini
# logger.conf
mode=async
level=debug
pattern=[%Y-%m-%d %H:%M:%S] [%l] [%f:%#] %m
console=true
colored_output=true
file=true
file_path=logs/application.log
queue_capacity=8192
overflow_policy=drop_oldest
flush_interval_ms=1000
```

Load it like this:

```cpp
#include "logger/logger.hpp"
#include "logger/parser.hpp"

int main() {
  auto config = logger::ConfigParser::parse_file("logger.conf");
  logger::Logger log{config};

  log.info("configured from file");
  log.flush();
}
```

Blank lines are ignored. Lines that start with `#` are comments. Unknown keys and invalid values are ignored.

Supported keys:

| Key | Values |
| --- | --- |
| `mode` | `sync`, `synchronous`, `async`, `asynchronous` |
| `level` | `trace`, `debug`, `info`, `warn`, `warning`, `error`, `fatal`, `off` |
| `pattern` | Any log pattern |
| `queue_size`, `queue_capacity` | Number |
| `overflow_policy` | `block`, `drop_newest`, `dropnewest`, `drop_oldest`, `dropoldest`, `discard` |
| `console`, `console_enabled` | Boolean |
| `file`, `file_enabled` | Boolean |
| `file_path` | Path |
| `rotation`, `rotation_enabled` | Boolean |
| `rotation_max_file_size` | Number of bytes |
| `rotation_max_files` | Number |
| `flush_interval`, `flush_interval_ms` | Milliseconds |
| `colored_output` | Boolean |
| `flush_on_error` | Boolean |

Boolean values can be `true`, `false`, `yes`, `no`, `on`, `off`, `1`, or `0`.

## Log Patterns

The pattern controls what each log line looks like. A newline is added automatically.

| Token | Meaning |
| --- | --- |
| `%Y-%m-%d %H:%M:%S` | Full local timestamp. |
| `%Y` | Year |
| `%d` | Day |
| `%H` | Hour |
| `%M` | Minute |
| `%S` | Second |
| `%l`, `%L` | Log level |
| `%t` | Thread id hash |
| `%p` | Process id |
| `%f` | Source file |
| `%F` | Function name |
| `%#` | Source line |
| `%n` | Logger name field |
| `%m`, `%v` | Log message |
| `%%` | Percent sign |

Common patterns:

```cpp
config.pattern = "[%Y-%m-%d %H:%M:%S] [%l] %m";
config.pattern = "[%l] [%f:%#] %m";
config.pattern = "[%p] [T%t] [%F] %m";
```

## Sinks

A sink is where logs are written.

By default, the logger adds a console sink if `enable_console` is true. It adds a normal file sink if `enable_file` is true.

You can also add sinks yourself:

```cpp
logger::Config config;
config.enable_console = false;

logger::Logger log{config};
log.add_sink<logger::ConsoleSink>(true);
log.add_sink<logger::FileSink>("logs/app.log");
```

For rotating files, add `RotatingFileSink` directly:

```cpp
logger::Config config;
config.enable_console = false;
config.enable_file = false;

logger::Logger log{config};
log.add_sink<logger::RotatingFileSink>("logs/app.log", 10 * 1024 * 1024, 5);
```

Sink types:

| Sink | Meaning |
| --- | --- |
| `ConsoleSink` | Writes to standard output. |
| `FileSink` | Appends to a file. |
| `RotatingFileSink` | Rotates files after a size limit. |
| `NullSink` | Drops every log message. Useful for tests and benchmarks. |

## Async Queue Behavior

Async logging uses a fixed-size queue. If the queue is full, the overflow policy decides what happens.

| Policy | Behavior |
| --- | --- |
| `OverflowPolicy::Block` | Wait until there is space. |
| `OverflowPolicy::DropNewest` | Drop the new message. |
| `OverflowPolicy::DropOldest` | Drop the oldest queued message and add the new one. |
| `OverflowPolicy::Discard` | Discard the new message. |

## Macros

You can include `logger/macros.hpp` for shorter calls:

```cpp
#include "logger/logger.hpp"
#include "logger/macros.hpp"

LOGGER_INFO(log, "user {} logged in", user_id);
LOGGER_ERROR(log, "failed to open {}", path);
```

These macros call the matching logger method.

## Project Layout

```text
include/logger/      Public headers
src/                 Library code
examples/            Example programs
tests/               Tests
benchmark/           Benchmarks
CMakeLists.txt       CMake build file
```

## Development

Use a separate build directory:

```sh
cmake -S . -B build -DLOGGER_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

To clean the normal build output:

```sh
rm -rf build
```

If CMake was run in the project root by mistake, remove these generated files:

```text
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
CTestTestfile.cmake
DartConfiguration.tcl
Makefile
```

## Use From Another CMake Project

Add this project and link to `logger::logger`:

```cmake
add_subdirectory(path/to/logparse)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE logger::logger)
```

The target adds the public include path and links `fmt::fmt`.
