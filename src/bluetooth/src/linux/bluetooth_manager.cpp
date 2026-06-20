// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.hpp"

#include <models/app_settings.hpp>
#include <models/bluetooth_packet.hpp>

#include "models/packet.hpp"
#include "unpacker/unpacker.hpp"

namespace screen_controller::bluetooth {

BluetoothManager::BluetoothManager(
    ILogger& logger, const AppSettings& settings,
    const std::function<void(const BluetoothPacket& packet)>& callback)
    : settings_(settings),
      logger_(logger),
      l2_cap_receiver_(logger, settings),
      dbus_manager_(dbus::DbusManager(logger)),
      connection_state_(ConnectionState::kStarting),
      bluetooth_callback_(callback) {
  l2_cap_receiver_.OnPacket([this](const Packet& raw) {
    BluetoothPacket packet{};
    packet.type = raw.type_;
    packet.name = raw.name_;

    if (raw.has_payload_) {
      const Unpacker unpacker(logger_);
      unpacker.Decompress(raw.payload_, packet);
    }

    bluetooth_callback_(packet);
  });

  logger_.LogInfo("Creating BluetoothManager");
}

std::expected<std::unique_ptr<BluetoothManager>, std::error_code> BluetoothManager::Create(
    ILogger& logger, const AppSettings& settings,
    const std::function<void(const BluetoothPacket& packet)>& callback) {
  try {
    auto bluetooth_manager =
        std::unique_ptr<BluetoothManager>(new BluetoothManager(logger, settings, callback));
    return bluetooth_manager;
  } catch (std::exception& e) {
    logger.LogError(std::string("BluetoothManager::Create failed: ") + e.what());
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::Poll() {
  l2_cap_receiver_.PollSocket();
}

}  // namespace screen_controller::bluetooth
