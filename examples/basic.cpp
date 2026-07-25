#include "logger/logger.hpp"

int main() {
  logger::Config config;
  config.mode = logger::LoggerMode::Sync;
  config.pattern = "[%Y-%m-%d %H:%M:%S] [%l] %m";

  logger::Logger log{config};
  log.info("logger initialized");
  log.warn("example warning");
  log.flush();
}
