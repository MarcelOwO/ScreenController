// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.h"

#include <ng-log/logging.h>

#include <fstream>
#include <iostream>
#include <string>

#include "unpacker/unpacker.h"

namespace screen_controller::bluetooth {
BluetoothManager::BluetoothManager() {
  LOG(INFO) << "Creating BluetoothManager";
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::on_packet_received(
    std::function<void(const common::BluetoothPacket& packet)> callback) {
  bluetooth_callback_ = std::move(callback);
}

bool BluetoothManager::init() {
  LOG(INFO) << "Initializing BluetoothManager";
  //CHECK(dbus_manager_->init()) << "Failed to init DBusManager";

  CHECK(l2_cap_receiver_.init()) << "Failed to initialize L2CAP receiver";

  l2_cap_receiver_.OnReceived([this](const std::span<std::byte> data) {
    common::BluetoothPacket packet{};
    Unpacker unpacker{};
    unpacker.init();
    unpacker.decompress(data, packet);
    bluetooth_callback_(packet);
  });

  return true;
}

void BluetoothManager::run() {
  l2_cap_receiver_.poll_socket();
  //dbus_manager_->poll_adapters();
}
}  // namespace screen_controller::bluetooth