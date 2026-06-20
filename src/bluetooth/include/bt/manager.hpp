#pragma once

#include <functional>

#include <logging/logger.hpp>
#include <models/app_settings.hpp>
#include <models/bluetooth_packet.hpp>

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
      const std::function<void(const BluetoothPacket& packet)>& callback);
};

}  // namespace screen_controller
