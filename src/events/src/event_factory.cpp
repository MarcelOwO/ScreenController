
#include <events/events.h>
#include <memory>

#include "app_settings.h"
#include "event_manager.h"
#include "logging/logger.h"

namespace screen_controller {

std::unique_ptr<IEventManager> EventFactory::Create(ILogger& logger, const AppSettings& settings) {
  auto manager = EventManager::Create(logger, settings);

  if (!manager) {
    throw std::runtime_error("Failed to create EventManager");
  }

  return std::move(manager.value());
}

}  // namespace screen_controller
