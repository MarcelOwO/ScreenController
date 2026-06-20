#include <bt/manager.hpp>

#include "bluetooth_manager.hpp"

namespace screen_controller {

std::unique_ptr<IBluetoothManager> BluetoothFactory::Create(ILogger& logger,
                                                             const AppSettings& settings,
                                                             IEventManager& events) {
  auto manager = bluetooth::BluetoothManager::Create(logger, settings, events);
  if (!manager) {
    throw std::runtime_error("Failed to create BluetoothManager");
  }
  return std::move(manager.value());
}

}  // namespace screen_controller
