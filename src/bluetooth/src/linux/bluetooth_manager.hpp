//
// Created by marce on 4/2/2025.
//

#pragma once

#include <bt/manager.hpp>

#include <events/events.hpp>
#include <logging/logger.hpp>
#include <models/app_settings.hpp>
#include <models/bluetooth_packet.hpp>

#include "dbus/dbus_manager.hpp"
#include "models/connection_state.hpp"
#include "socket/l2cap_receiver.hpp"

namespace screen_controller::bluetooth {

class BluetoothManager : public IBluetoothManager {
public:
  ~BluetoothManager() override;

  static std::expected<std::unique_ptr<BluetoothManager>, std::error_code> Create(
      ILogger& logger, const AppSettings& settings, IEventManager& events);

  BluetoothManager(const BluetoothManager&) = delete;
  BluetoothManager& operator=(const BluetoothManager&) = delete;
  BluetoothManager(BluetoothManager&&) = delete;
  BluetoothManager& operator=(BluetoothManager&&) = delete;

  void Poll() override;
  bool SendPacket(uint8_t type, std::string_view name,
                  std::span<const std::byte> payload = {}) override;

private:
  explicit BluetoothManager(ILogger& logger, const AppSettings& settings, IEventManager& events);

  const AppSettings& settings_;
  ILogger& logger_;
  IEventManager& events_;

  socket::L2CapReceiver l2_cap_receiver_;
  dbus::DbusManager dbus_manager_;

  ConnectionState connection_state_;
};

}  // namespace screen_controller::bluetooth
