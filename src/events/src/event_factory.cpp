
#include <events/events.h>
#include <memory>
#include "app_settings.h"
#include "logging/logger.h"

namespace screen_controller {

std::unique_ptr<IMediatorManager> MediatorFactory::Create(ILogger& logger,
                                                          const AppSettings& settings) {
  auto manager = MediatorManager::Create(logger, settings);

  if (!manager) {
    throw std::runtime_error("Failed to create MediatorManager");
  }
  return std::move(manager.value());
}

}  // namespace screen_controller
