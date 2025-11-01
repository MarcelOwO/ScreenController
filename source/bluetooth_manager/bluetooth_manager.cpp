// Created by marce on 4/2/2025.
//

#include <bluetooth_manager/bluetooth_manager.h>

#include "app_settings.h"
#include "unpacker/unpacker.h"

namespace screen_controller {

BluetoothManager::BluetoothManager(const std::shared_ptr<Logger>& logger, const std::shared_ptr<AppSettings>& settings)
    : settings_(settings), logger_(logger), l2_cap_receiver_(logger),
      dbus_manager_(logger,settings->app_name) {
  logger_->LogInfo("Creating BluetoothManager");

  if (!l2_cap_receiver_.init()) {
    logger_->LogError("Failed to init L2CAP receiver");
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

void BluetoothManager::on_packet_received(
    std::function<void(const common::BluetoothPacket& packet)> callback) {
  bluetooth_callback_ = std::move(callback);
}

void BluetoothManager::run() {
  l2_cap_receiver_.poll_socket();
  dbus_manager_.poll_adapters();
}
}  // namespace screen_controller
