//
// Created by marce on 5/3/2025.
//

#include "bluetooth_agent_manager.h"

#include <iostream>
#include <utility>

#include "sdbus-c++/IProxy.h"
namespace screen_controller::dbus {
BluetoothAgentManager::BluetoothAgentManager(
    std::shared_ptr<sdbus::IProxy> bluez_proxy)
    : bluez_proxy_(std::move(bluez_proxy)),
      agent_manager_interface_name_(
          sdbus::InterfaceName("org.bluez.AgentManager1")) {}

bool BluetoothAgentManager::RegisterAgent(sdbus::ObjectPath agent,
                                          std::string capability) const {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("RegisterAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent, capability);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}
bool BluetoothAgentManager::UnregisterAgent(sdbus::ObjectPath agent) const {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("UnregisterAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}
bool BluetoothAgentManager::RequestDefaultAgent(sdbus::ObjectPath agent) {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("RequestDefaultAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}
}  // namespace screen_controller::dbus