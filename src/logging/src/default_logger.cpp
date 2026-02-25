#include "default_logger.h"

#include <print>

namespace screen_controller {

DefaultLogger::DefaultLogger() {}

DefaultLogger::~DefaultLogger() {};

void DefaultLogger::Log(const LogLevel level, const std::string_view log) {
  switch (level) {
    case LogLevel::INFO: {
      std::println("{}", log);
      break;
    }
    case LogLevel::ERROR: {
      std::println("{}", (log));
      break;
    }
    default: {
      std::println("Unknown log level");
      break;
    }
  }
}

}  // namespace screen_controller
