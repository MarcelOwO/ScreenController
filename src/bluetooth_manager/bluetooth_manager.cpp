// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.h"

#include <ng-log/logging.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "dbus_implementations/bluetooth_adapter.h"
#include "dbus_implementations/bluetooth_agent.h"
#include "dbus_implementations/bluetooth_agent_manager.h"
#include "dbus_implementations/bluetooth_le_advertisement.h"
#include "dbus_implementations/bluetooth_le_advertising_manager.h"
#include "unpacker/unpacker.h"

namespace screen_controller::bluetooth {
BluetoothManager::BluetoothManager()
    : advertisement_path_(sdbus::ObjectPath("/org/bluez/hci0/owo")),
      agent_path_(sdbus::ObjectPath("/org/bluez/hci0/owo/agent1")) {
  LOG(INFO) << "Creating BluetoothManager";
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::on_blutooth_packet_received(
    std::function<void(common::BluetoothPacket)> callback) {
  bluetooth_callback_ = std::move(callback);
}

bool BluetoothManager::init() {
  connection_ = sdbus::createSystemBusConnection();
  CHECK(connection_ != nullptr) << "Failed to create system bus connection";

  adapter_proxy_ =
      sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                         sdbus::ObjectPath("/org/bluez/hci0"));

  CHECK(adapter_proxy_ != nullptr) << "Failed to create adapter proxy";

  bluez_proxy =
      sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                         sdbus::ObjectPath("/org/bluez"));

  CHECK(bluez_proxy != nullptr) << "Failed to create bluez proxy";

  try {
    dbus::BluetoothAdapter adapter(adapter_proxy_);
    CHECK(adapter.init()) << "Failed to initialize adapter";

    const dbus::BluetoothLeAdvertisingManager advertising_manager(
        adapter_proxy_);

    const dbus::BluetoothAgent agent(connection_, agent_path_);
    agent.init();

    const std::string capabilities = "NoInputNoOutput";
    dbus::BluetoothAgentManager agent_manager(bluez_proxy);

    CHECK(agent_manager.RegisterAgent(agent_path_, capabilities))
        << "Failed to register agent";

    CHECK(agent_manager.RequestDefaultAgent(agent_path_))
        << "Failed to request default agent";

    const dbus::BluetoothLEAdvertisement advertisement(connection_,
                                                       advertisement_path_);

    advertisement.init();

    CHECK(advertising_manager.RegisterAdvertisement(
        advertisement_path_, std::unordered_map<std::string, sdbus::Variant>()))
        << "Failed to register advertisement";

  } catch (const sdbus::Error& e) {
    LOG(ERROR) << e.what();
    return false;
  }

  connection_->enterEventLoopAsync();

  last_time_point_ = std::chrono::steady_clock::now();

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
  const auto current_time = std::chrono::steady_clock::now();

  const auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
      current_time - last_time_point_);

  l2_cap_receiver_.run();

  if (elapsed_time.count() < 50) {
    return;
  }

  last_time_point_ = current_time;

  dbus::BluetoothAdapter adapter(adapter_proxy_);

  if (!adapter.init()) {
    LOG(ERROR) << "Failed to initialize adapter";
  }
}
}  // namespace screen_controller::bluetooth