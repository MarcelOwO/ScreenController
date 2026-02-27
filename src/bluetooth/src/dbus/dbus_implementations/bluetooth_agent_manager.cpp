//
// Created by marce on 5/3/2025.
//

#include "bluetooth_agent_manager.h"

#include <logging/logger.h>

#include <iostream>
#include <utility>

#include "sdbus-c++/IProxy.h"
namespace screen_controller {
BluetoothAgentManager::BluetoothAgentManager(
    ILogger& logger, const std::shared_ptr<sdbus::IProxy>& bluez_proxy)
    : logger_(logger),
      bluez_proxy_(bluez_proxy),
      agent_manager_interface_name_(
          sdbus::InterfaceName("org.bluez.AgentManager1")) {
  logger_.LogInfo("Creating BluetoothAgentManager");
}

std::expected<void, std::error_code> BluetoothAgentManager::RegisterAgent(
    const sdbus::ObjectPath& agent, std::string_view capability) const {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("RegisterAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent, capability);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothAgentManager::UnregisterAgent(
    const sdbus::ObjectPath& agent) const {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("UnregisterAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothAgentManager::RequestDefaultAgent(
    const sdbus::ObjectPath& agent) const {
  try {
    (void)bluez_proxy_->callMethod(sdbus::MethodName("RequestDefaultAgent"))
        .onInterface(agent_manager_interface_name_)
        .withArguments(agent);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}
}  // namespace screen_controller
