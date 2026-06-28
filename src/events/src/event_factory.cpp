#include <events/events.hpp>
#include <helper/unwrap.hpp>
#include <logging/logger.hpp>
#include <memory>
#include <models/app_settings.hpp>

#include "event_manager.hpp"

namespace screen_controller {

std::unique_ptr<IEventManager> EventFactory::Create(ILogger& logger, const AppSettings& settings) {
  return Unwrap(EventManager::Create(logger, settings), "Failed to create EventManager");
}

}  // namespace screen_controller
