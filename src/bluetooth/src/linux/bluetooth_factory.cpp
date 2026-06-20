#include <bt/manager.h>

#include "bluetooth_manager.h"

namespace screen_controller {

std::unique_ptr<IBluetoothManager> BluetoothFactory::Create(
    ILogger& logger, const AppSettings& settings,
    const std::function<void(const common::BluetoothPacket& packet)>& callback) {
  auto manager = bluetooth::BluetoothManager::Create(logger, settings, callback);
  if (!manager) {
    throw std::runtime_error("Failed to create BluetoothManager");
  }
  return std::move(manager.value());
}

}  // namespace screen_controller
