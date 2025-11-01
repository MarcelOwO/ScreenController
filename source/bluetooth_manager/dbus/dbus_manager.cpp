//
// Created by marce on 05/08/2025.
//

#include "dbus_manager.h"

#include "dbus_implementations/bluetooth_adapter.h"
#include "dbus_implementations/bluetooth_agent.h"
#include "dbus_implementations/bluetooth_agent_manager.h"
#include "dbus_implementations/bluetooth_le_advertisement.h"
#include "dbus_implementations/bluetooth_le_advertising_manager.h"
#include "logging/logger.h"
#include "sdbus-c++/Types.h"
#include <string_view>

namespace screen_controller {
DbusManager::DbusManager(const std::shared_ptr<Logger>& logger, std::string_view alias)
    : alias_(alias),
      logger_(logger),
      advertisement_path_({sdbus::ObjectPath("/org/bluez/hci0/owo")}),
      agent_path_({sdbus::ObjectPath("/org/bluez/hci0/owo/agent1")})

{
  logger_->LogInfo("Creating DBusManager");
  last_time_point_ = std::chrono::steady_clock::now();
  connection_ = sdbus::createSystemBusConnection();

  adapter_proxy_ =
      sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                         sdbus::ObjectPath("/org/bluez/hci0"));

  bluez_proxy_ =
      sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                         sdbus::ObjectPath("/org/bluez"));

  setup_adapter();

  const BluetoothLeAdvertisingManager advertising_manager(logger_,
                                                          adapter_proxy_);
  const BluetoothAgent agent(logger_, connection_, agent_path_);

  const std::string capabilities = "NoInputNoOutput";

  BluetoothAgentManager agent_manager(logger_, bluez_proxy_);

  if (!agent_manager.RegisterAgent(agent_path_, capabilities)) {
    logger_->LogError("Failed to register agent");
  }

  if (!agent_manager.RequestDefaultAgent(agent_path_)) {
    logger_->LogError("Failed to request default agent");
  }

  const BluetoothLEAdvertisement advertisement(logger_, connection_,
                                               advertisement_path_);

  if (!advertising_manager.RegisterAdvertisement(
          advertisement_path_,
          std::unordered_map<std::string, sdbus::Variant>())) {
    logger_->LogError("Failed to register advertisement");
    return;
  }

  connection_->enterEventLoopAsync();
}

void DbusManager::poll_adapters() {
  const auto current_time = std::chrono::steady_clock::now();

  const auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
      current_time - last_time_point_);

  if (elapsed_time.count() < 50) {
    return;
  }

  last_time_point_ = current_time;

  setup_adapter();
}

void DbusManager::setup_adapter() const {
  BluetoothAdapter adapter(logger_, adapter_proxy_);

  const auto res = adapter.get_alias();

  if (!res) {
    logger_->LogError("Failed to get alias name");
    return;
  }

  if (const auto current_alias = res.value(); current_alias != alias_) {
    const auto res = adapter.set_alias(alias_);

    if (!res) {
      logger_->LogError("Failed to set alias");
      return;
    }
  }

  if (!adapter.get_powered()) {
    if (!adapter.set_powered(true)) {
      logger_->LogError("Failed to set powered");
    }
  }

  if (!adapter.get_discoverable()) {
    if (!adapter.set_discoverable(true)) {
      logger_->LogError("Failed to set discoverable");
    }
  }

  if (!adapter.get_pairable()) {
    if (adapter.set_pairable(true)) {
      logger_->LogError("Failed to set pairable");
    }
  }
}

}  // namespace screen_controller
