// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.h"

#include "app_settings.h"
#include "unpacker/unpacker.h"

namespace screen_controller {

BluetoothManager::BluetoothManager(
    ILogger& logger, const AppSettings& settings,
    const std::function<void(const common::BluetoothPacket& packet)>& callback)
    : settings_(settings),
      logger_(logger),
      l2_cap_receiver_(logger, settings),
      dbus_manager_(DbusManager(logger)),
      connection_state_(ConnectionState::kStarting),
      bluetooth_callback_(callback) {
  if (!l2_cap_receiver_.Init()) {
    logger.LogError("Failed to init L2CAP receiver");
    return;
  }

  l2_cap_receiver_.OnReceived([&](const std::span<std::byte> kData) {
    common::BluetoothPacket packet{};
    const Unpacker kUnpacker(logger);
    kUnpacker.Decompress(kData, packet);
    bluetooth_callback_(packet);
  });

  logger_.LogInfo("Creating BluetoothManager");
}

std::expected<std::unique_ptr<BluetoothManager>, std::error_code> BluetoothManager::Create(
    ILogger& logger, const AppSettings& settings,
    const std::function<void(const common::BluetoothPacket& packet)>& callback) {
  try {
  } catch (std::exception& e) {
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  auto bluetooth_manager =
      std::unique_ptr<BluetoothManager>(new BluetoothManager(logger, settings, callback));

  return bluetooth_manager;
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::Poll() {
  l2_cap_receiver_.PollSocket();
}
}  // namespace screen_controller
