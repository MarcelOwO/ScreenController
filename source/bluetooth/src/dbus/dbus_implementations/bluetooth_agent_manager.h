//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_AGENT_MANAGER_H
#define BLUETOOTH_AGENT_MANAGER_H
#include <logging/logger.h>

#include <expected>
#include <memory>

#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

<<<<<<<< HEAD:source/bluetooth_manager/dbus/dbus_implementations/bluetooth_agent_manager.h
namespace screen_controller {
class BluetoothAgentManager {
 public:
  explicit BluetoothAgentManager(
      const std::shared_ptr<Logger>& logger,
      const std::shared_ptr<sdbus::IProxy>& adapter_proxy);
========
namespace screen_controller::dbus {
class BluetoothAgentManager {
public:
  explicit BluetoothAgentManager(ILogger& logger, sdbus::IProxy& adapter_proxy);
>>>>>>>> origin/dev:source/bluetooth/src/dbus/dbus_implementations/bluetooth_agent_manager.h
  ~BluetoothAgentManager() = default;

  BluetoothAgentManager(const BluetoothAgentManager&) = delete;
  BluetoothAgentManager& operator=(const BluetoothAgentManager&) = delete;
  BluetoothAgentManager(BluetoothAgentManager&&) = delete;
  BluetoothAgentManager& operator=(BluetoothAgentManager&&) = delete;

<<<<<<<< HEAD:source/bluetooth_manager/dbus/dbus_implementations/bluetooth_agent_manager.h
  [[nodiscard]] std::expected<void, std::error_code> RegisterAgent(
      const sdbus::ObjectPath& agent, std::string_view capability) const;
  [[nodiscard]] std::expected<void, std::error_code> UnregisterAgent(
      const sdbus::ObjectPath& agent) const;
  [[nodiscard]] std::expected<void, std::error_code> RequestDefaultAgent(
      const sdbus::ObjectPath& agent) const;

 private:
  std::shared_ptr<Logger> logger_;
  std::shared_ptr<sdbus::IProxy> bluez_proxy_;
========
  [[nodiscard]] std::expected<void, std::error_code> RegisterAgent(const sdbus::ObjectPath& agent,
                                                                   std::string_view capability);
  [[nodiscard]] std::expected<void, std::error_code> UnregisterAgent(
      const sdbus::ObjectPath& agent);
  [[nodiscard]] std::expected<void, std::error_code> RequestDefaultAgent(
      const sdbus::ObjectPath& agent);

private:
  ILogger& logger_;
  sdbus::IProxy& bluez_proxy_;
>>>>>>>> origin/dev:source/bluetooth/src/dbus/dbus_implementations/bluetooth_agent_manager.h
  sdbus::InterfaceName agent_manager_interface_name_;
};
}  // namespace screen_controller

#endif  // BLUETOOTH_AGENT_MANAGER_H
