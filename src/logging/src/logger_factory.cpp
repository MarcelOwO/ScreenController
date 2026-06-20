#include "logging/logger.hpp"

#include "default_logger.hpp"

namespace screen_controller {

std::unique_ptr<ILogger> LoggerFactory::Create() {
  auto logger = DefaultLogger::Create();

  if (!logger) {
    throw std::runtime_error("failed to create logger");
  }

  return std::move(logger.value());
};

}  // namespace screen_controller
