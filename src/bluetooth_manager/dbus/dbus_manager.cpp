//
// Created by marce on 05/08/2025.
//

#include "dbus_implementations/bluetooth_adapter.h"
#include "dbus_implementations/bluetooth_agent.h"
#include "dbus_implementations/bluetooth_agent_manager.h"
#include "dbus_implementations/bluetooth_le_advertisement.h"
#include "dbus_implementations/bluetooth_le_advertising_manager.h"
#include "dbus_manager.h"
#include "ng-log/logging.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::bluetooth::dbus {
DbusManager::DbusManager(){
  LOG(INFO) << "Creating DBusManager";
}

bool DbusManager::init() {
  LOG(INFO) << "Initializing DBusManager";
    connection_ = sdbus::createSystemBusConnection();
  CHECK(connection_ != nullptr) << "Failed to create system bus connection";

  adapter_proxy_ =
      sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                         sdbus::ObjectPath("/org/bluez/hci0"));
  CHECK(adapter_proxy_ != nullptr) << "Failed to create adapter proxy";

  bluez_proxy_ =
      sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                         sdbus::ObjectPath("/org/bluez"));

  CHECK(bluez_proxy_ != nullptr) << "Failed to create bluez proxy";

  BluetoothAdapter adapter(adapter_proxy_);

  CHECK(adapter.init()) << "Failed to initialize adapter";
  const BluetoothLeAdvertisingManager advertising_manager(adapter_proxy_);
  const BluetoothAgent agent(connection_, agent_path_);
  agent.init();

  const std::string capabilities = "NoInputNoOutput";
  BluetoothAgentManager agent_manager(bluez_proxy_);

  CHECK(agent_manager.RegisterAgent(agent_path_, capabilities))
      << "Failed to register agent";

  CHECK(agent_manager.RequestDefaultAgent(agent_path_))
      << "Failed to request default agent";

  const BluetoothLEAdvertisement advertisement(connection_,
                                               advertisement_path_);

  advertisement.init();

  CHECK(advertising_manager.RegisterAdvertisement(
      advertisement_path_, std::unordered_map<std::string, sdbus::Variant>()))
      << "Failed to register advertisement";

  connection_->enterEventLoopAsync();
  return true;
}

void DbusManager::poll_adapters() {
  const auto current_time = std::chrono::steady_clock::now();

  const auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
      current_time - last_time_point_);

  if (elapsed_time.count() < 50) {
    return;
  }

  last_time_point_ = current_time;

  BluetoothAdapter adapter(adapter_proxy_);

  if (!adapter.init()) {
    LOG(ERROR) << "Failed to initialize adapter";
  }
}

}  // namespace screen_controller::bluetooth::dbus