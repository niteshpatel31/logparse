#include "../include/logger/config.hpp"
#include <cstdlib>
#include <fmt/color.h>

int main() {
  fmt::println("{}", static_cast<bool>(logger::LoggerMode::Async));
  fmt::println("{}", static_cast<bool>(logger::LoggerMode::Sync));
  return EXIT_SUCCESS;
}
