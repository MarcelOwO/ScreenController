
#include <events/events.hpp>
#include <memory>

#include <logging/logger.hpp>
#include <models/app_settings.hpp>
#include "event_manager.hpp"

namespace screen_controller {

std::unique_ptr<IEventManager> EventFactory::Create(ILogger& logger, const AppSettings& settings) {
  auto manager = EventManager::Create(logger, settings);

  if (!manager) {
    throw std::runtime_error("Failed to create EventManager");
  }

  return std::move(manager.value());
}

}  // namespace screen_controller
