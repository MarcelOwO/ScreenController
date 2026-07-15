//
// Created by marce on 05/08/2025.
//

#pragma once

#include <logging/logger.hpp>

#include <chrono>
#include <memory>

#include "dbus_implementations/bluetooth_adapter.hpp"
#include "dbus_implementations/bluetooth_agent.hpp"
#include "dbus_implementations/bluetooth_agent_manager.hpp"
#include "dbus_implementations/bluetooth_le_advertisement.hpp"
#include "dbus_implementations/bluetooth_le_advertising_manager.hpp"
#include "dbus_implementations/gatt_application.hpp"
#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::dbus {

class DbusManager {
public:
  explicit DbusManager(ILogger& logger);

  ~DbusManager();

  DbusManager(const DbusManager&) = delete;
  DbusManager& operator=(const DbusManager&) = delete;
  DbusManager(DbusManager&&) = delete;
  DbusManager& operator=(DbusManager&&) = delete;

  void MakeConnectable(bool allow_pairing);
  void DisableNewConnection();
  void Poll();
  [[nodiscard]] GattApplication& Gatt() noexcept;

private:
  std::string alias_;
  ILogger& logger_;

  std::shared_ptr<sdbus::IConnection> connection_;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  std::shared_ptr<sdbus::IProxy> bluez_proxy_;

  sdbus::ObjectPath advertisement_path_;
  sdbus::ObjectPath agent_path_;

  BluetoothAgentManager agent_manager_;
  BluetoothAgent agent_;
  BluetoothAdapter adapter_;
  BluetoothLeAdvertisingManager advertising_manager_;
  BluetoothLEAdvertisement advertisement_;
  GattApplication gatt_application_;
  bool connectable_requested_{false};
  std::chrono::steady_clock::time_point next_advertisement_attempt_{};

  void SetupAgents();
  void SetupName();
  void SetupAdapter();
};
}  // namespace screen_controller::dbus
