#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <bluetooth_packet.h>
#include <logging/logger.h>

#include <functional>

#include "app_settings.h"

namespace screen_controller {

class IBluetoothManager {
public:
  virtual ~IBluetoothManager() = default;

  virtual void Poll() = 0;
};

class BluetoothFactory {
public:
  static std::unique_ptr<IBluetoothManager> Create(
      ILogger& logger, const AppSettings& settings,
      const std::function<void(const common::BluetoothPacket& packet)>& callback);
};

}  // namespace screen_controller

#endif  //
