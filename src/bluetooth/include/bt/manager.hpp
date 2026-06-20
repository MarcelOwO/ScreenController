#pragma once

#include <events/events.hpp>
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
  static std::unique_ptr<IBluetoothManager> Create(ILogger& logger, const AppSettings& settings,
                                                   IEventManager& events);
};

}  // namespace screen_controller
