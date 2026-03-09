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
      l2_cap_receiver_(logger),
      dbus_manager_(logger),
      connection_state_(ConnectionState::STARTING),
      bluetooth_callback_(callback) {
  logger_.LogInfo("Creating BluetoothManager");

  if (!l2_cap_receiver_.init()) {
    logger_.LogError("Failed to init L2CAP receiver");
    return;
  }

  l2_cap_receiver_.OnReceived([&](const std::span<std::byte> data) {
    common::BluetoothPacket packet{};
    const Unpacker unpacker(logger);
    unpacker.decompress(data, packet);
    bluetooth_callback_(packet);
  });
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::poll() { l2_cap_receiver_.poll_socket(); }
}  // namespace screen_controller
