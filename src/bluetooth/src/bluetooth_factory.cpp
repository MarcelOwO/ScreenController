#include <bt/manager.h>

#include "bluetooth_manager.h"

namespace screen_controller {

std::unique_ptr<IBluetoothManager> BluetoothFactory::Create(
    ILogger& logger, const AppSettings& settings) {
  return std::make_unique<BluetoothManager>(logger, settings);
}

}  // namespace screen_controller
