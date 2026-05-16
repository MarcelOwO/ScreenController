//
// Created by marce on 05/08/2025.
//

#include "dbus_manager.h"

#include "dbus_implementations/bluetooth_adapter.h"
#include "dbus_implementations/bluetooth_agent.h"
#include "dbus_implementations/bluetooth_agent_manager.h"
#include "logging/logger.h"
#include "sdbus-c++/sdbus-c++.h"

namespace screen_controller::dbus {
DbusManager::DbusManager(ILogger& logger) try
    : logger_(logger),

      service_name_(sdbus::ServiceName("com.owo.screen_controller")),
      agent_path_(sdbus::ObjectPath("/owo/agent1")),
      connection_(sdbus::createSystemBusConnection(service_name_)),

      adapter_proxy_(sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                                        sdbus::ObjectPath("/org/bluez/hci0"))),
      bluez_proxy_(sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                                      sdbus::ObjectPath("/org/bluez"))),
      agent_(logger_, connection_, agent_path_),
      agent_manager_(logger_, *bluez_proxy_),
      adapter_(logger_, adapter_proxy_) {
  logger_.LogInfo("Creating DBusManager");

  // test

  connection_->requestName(service_name_);

  if (!agent_manager_.RegisterAgent(agent_path_, "NoInputNoOutput")) {
    logger_.LogError("Failed to register agent");
  }

  if (!agent_manager_.RequestDefaultAgent(agent_path_)) {
    logger_.LogError("Failed to request default agent");
  }

  SetupAdapter();

  connection_->enterEventLoopAsync();
} catch (std::exception& e) {
  auto format = std::format("Failed to create DBusManager: {}", e.what());

  logger.LogError(format);

  throw;
}

DbusManager::~DbusManager() {
  connection_->leaveEventLoop();
}

void DbusManager::SetupName() {
  const auto kAdapterAlias = adapter_.get_alias();

  if (!kAdapterAlias) {
    logger_.LogError("Failed to get alias name");
    return;
  }

  const auto kCurrentAlias = kAdapterAlias.value();

  if (kCurrentAlias == alias_) {
    logger_.LogError("Alias is already set");
    return;
  }

  const auto kRes = adapter_.set_alias(alias_);

  if (!kRes) {
    logger_.LogError("Failed to set alias");
    return;
  }
}

void DbusManager::MakeConnectable() {
  if (!adapter_.set_discoverable(true)) {
    logger_.LogError("Failed to set discoverable");
  }
  if (adapter_.set_pairable(true)) {
    logger_.LogError("Failed to set pairable");
  }
}

void DbusManager::DisableNewConnection() {
  if (!adapter_.set_discoverable(false)) {
    logger_.LogError("Failed to set discoverable");
  }
  if (adapter_.set_pairable(false)) {
    logger_.LogError("Failed to set pairable");
  }
}

void DbusManager::SetupAdapter() {
  if (!adapter_.get_powered()) {
    if (!adapter_.set_powered(true)) {
      logger_.LogError("Failed to set powered");
    }
  }

  if (auto res = adapter_.set_pairable_timeout(0); !res) {
    logger_.LogError("Failed to set pairable timeout");
  }

  if (auto res = adapter_.set_discoverable_timeout(0); !res) {
    logger_.LogError("Failed to set discoverable timeout");
  }
}

}  // namespace screen_controller::dbus
