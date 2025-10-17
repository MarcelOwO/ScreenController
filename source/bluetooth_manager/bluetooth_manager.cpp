// Created by marce on 4/2/2025.
//

#include "include/bluetooth_manager/bluetooth_manager.h"

#include <fstream>
#include <iostream>
#include <string>

#include "unpacker/unpacker.h"

namespace screen_controller::bluetooth {
BluetoothManager::BluetoothManager(std::shared_ptr<Logger> logger)
    : logger_(logger) {
  logger_->LogInfo("Creating BluetoothManager");
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::on_packet_received(
    std::function<void(const common::BluetoothPacket& packet)> callback) {
  bluetooth_callback_ = std::move(callback);
}

std::expected<void, common::ErrorEnum> BluetoothManager::init() {
  logger_->LogInfo("Initializing BluetoothManager");
  if (!dbus_manager_->init()) {
    logger_->LogError("Failed to init DBusManager");
    return std::unexpected(common::ErrorEnum::ERROR);
  }
  if (!l2_cap_receiver_.init()) {
    logger_->LogError("Failed to init L2CAP receiver");
    return std::unexpected(common::ErrorEnum::ERROR);
  }


  l2_cap_receiver_.OnReceived([this](const std::span<std::byte> data) {
    common::BluetoothPacket packet{};
    Unpacker unpacker{};
    unpacker.init();
    unpacker.decompress(data, packet);
    bluetooth_callback_(packet);
  });

  return {};
}

void BluetoothManager::run() {
  l2_cap_receiver_.poll_socket();
  // dbus_manager_->poll_adapters();
}
}  // namespace screen_controller::bluetooth
