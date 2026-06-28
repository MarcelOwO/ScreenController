#include "logging/logger.hpp"

#include <helper/unwrap.hpp>

#include "default_logger.hpp"

namespace screen_controller {

std::unique_ptr<ILogger> LoggerFactory::Create() {
  return Unwrap(DefaultLogger::Create(), "Failed to create logger");
}

}  // namespace screen_controller
