#pragma once

#include <events/events.hpp>
#include <logging/logger.hpp>
#include <models/app_settings.hpp>
#include <models/bluetooth_packet.hpp>

#include <span>
#include <string_view>

namespace screen_controller {

class IBluetoothManager {
public:
  virtual ~IBluetoothManager() = default;

  virtual void Poll() = 0;
  virtual bool SendPacket(uint8_t type, std::string_view name,
                          std::span<const std::byte> payload = {}) = 0;
};

class BluetoothFactory {
public:
  static std::unique_ptr<IBluetoothManager> Create(ILogger& logger, const AppSettings& settings,
                                                   IEventManager& events);
};

}  // namespace screen_controller
