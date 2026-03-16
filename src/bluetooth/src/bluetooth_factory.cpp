#include <bt/manager.h>

#include "bluetooth_manager.h"

namespace screen_controller {

std::unique_ptr<IBluetoothManager> BluetoothFactory::Create(
    ILogger& logger, const AppSettings& settings,
    const std::function<void(const common::BluetoothPacket& packet)>& callback) {
  return std::make_unique<BluetoothManager>(logger, settings, callback);
}

}  // namespace screen_controller
