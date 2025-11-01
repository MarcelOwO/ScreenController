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

namespace screen_controller {
class BluetoothAgentManager {
 public:
  explicit BluetoothAgentManager(
      const std::shared_ptr<Logger>& logger,
      const std::shared_ptr<sdbus::IProxy>& adapter_proxy);
  ~BluetoothAgentManager() = default;

  BluetoothAgentManager(const BluetoothAgentManager&) = delete;
  BluetoothAgentManager& operator=(const BluetoothAgentManager&) = delete;
  BluetoothAgentManager(BluetoothAgentManager&&) = delete;
  BluetoothAgentManager& operator=(BluetoothAgentManager&&) = delete;

  [[nodiscard]] std::expected<void, std::error_code> RegisterAgent(
      const sdbus::ObjectPath& agent, std::string_view capability) const;
  [[nodiscard]] std::expected<void, std::error_code> UnregisterAgent(
      const sdbus::ObjectPath& agent) const;
  [[nodiscard]] std::expected<void, std::error_code> RequestDefaultAgent(
      const sdbus::ObjectPath& agent) const;

 private:
  std::shared_ptr<Logger> logger_;
  std::shared_ptr<sdbus::IProxy> bluez_proxy_;
  sdbus::InterfaceName agent_manager_interface_name_;
};
}  // namespace screen_controller

#endif  // BLUETOOTH_AGENT_MANAGER_H
