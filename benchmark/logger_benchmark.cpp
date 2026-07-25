#include "logger/logger.hpp"
#include "logger/sink.hpp"

#include <benchmark/benchmark.h>

namespace {

void BM_sync_null_sink(benchmark::State &state) {
  logger::Config config;
  config.mode = logger::LoggerMode::Sync;
  config.enable_console = false;

  logger::Logger log{config};
  log.add_sink<logger::NullSink>();

  for (auto _ : state) {
    log.info("benchmark message");
  }
}

} // namespace

BENCHMARK(BM_sync_null_sink);
BENCHMARK_MAIN();
