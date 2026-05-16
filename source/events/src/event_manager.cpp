#include "event_manager.h"

namespace screen_controller {

std::expected<std::unique_ptr<EventManager>, std::error_code> EventManager::Create(
    ILogger& logger, const AppSettings& settings) {
  auto event_manager = std::unique_ptr<EventManager>(new EventManager(logger, settings));

  return event_manager;
}

EventManager::~EventManager() {
  logger_.LogInfo("Cleaning up Event Manager");
}

EventManager::EventManager(ILogger& logger, const AppSettings& settings)
    : settings_(settings), logger_(logger), registry_({}) {}

void EventManager::InternalPublish(std::type_index type, const Message& msg) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto itr = registry_.find(type);
  if (itr != registry_.end()) {
    for (const auto& handler : itr->second) {
      handler(msg);
    }
  }
}
void EventManager::InternalSubscribe(std::type_index type,
                                     std::function<void(const Message&)> msg_callback) {
  std::lock_guard<std::mutex> lock(mutex_);

  registry_[type].push_back(std::move(msg_callback));
}

}  // namespace screen_controller
