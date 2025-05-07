//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_AGENT_MANAGER_H
#define BLUETOOTH_AGENT_MANAGER_H
#include <memory>

#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::dbus {
class BluetoothAgentManager {
 public:
  BluetoothAgentManager(std::shared_ptr<sdbus::IProxy> adapter_proxy);
  ~BluetoothAgentManager() = delete;

  BluetoothAgentManager(const BluetoothAgentManager&) = delete;
  BluetoothAgentManager& operator=(const BluetoothAgentManager&) = delete;
  BluetoothAgentManager(BluetoothAgentManager&&) = delete;
  BluetoothAgentManager& operator=(BluetoothAgentManager&&) = delete;

  bool RegisterAgent(sdbus::ObjectPath agent, std::string capability) const;
  bool UnregisterAgent(sdbus::ObjectPath agent) const;
  bool RequestDefaultAgent(sdbus::ObjectPath agent);

 private:
  std::shared_ptr<sdbus::IProxy> bluez_proxy_;
  sdbus::InterfaceName agent_manager_interface_name_;
};
}  // namespace screen_controller::dbus

#endif  // BLUETOOTH_AGENT_MANAGER_H
