//
// Created by marce on 5/3/2025.
//

#include "bluetooth_agent_manager.h"

#include <ng-log/logging.h>

#include <iostream>
#include <utility>

#include "sdbus-c++/IProxy.h"
namespace screen_controller::bluetooth::dbus {
BluetoothAgentManager::BluetoothAgentManager(
    const std::shared_ptr<sdbus::IProxy>& bluez_proxy)
    : bluez_proxy_(bluez_proxy),
      agent_manager_interface_name_(
          sdbus::InterfaceName("org.bluez.AgentManager1")) {
  LOG(INFO) << "Creating BluetoothAgentManager";
}

bool BluetoothAgentManager::RegisterAgent(const sdbus::ObjectPath& agent,
                                          std::string_view capability) const {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("RegisterAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent, capability);
    return true;
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAgentManager::UnregisterAgent(const sdbus::ObjectPath& agent) const {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("UnregisterAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent);
    return true;
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAgentManager::RequestDefaultAgent(const sdbus::ObjectPath& agent) {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("RequestDefaultAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent);
    return true;
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}
}  // namespace screen_controller::dbus