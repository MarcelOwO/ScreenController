//
// Created by marce on 05/08/2025.
//

#include "dbus_manager.hpp"

#include <logging/logger.hpp>
#include "dbus_implementations/bluetooth_adapter.hpp"
#include "dbus_implementations/bluetooth_agent.hpp"
#include "dbus_implementations/bluetooth_agent_manager.hpp"
#include "sdbus-c++/sdbus-c++.h"

namespace screen_controller::dbus {
DbusManager::DbusManager(ILogger& logger) try
    : alias_("ScreenController"),
      logger_(logger),
      connection_(sdbus::createSystemBusConnection()),
      adapter_proxy_(sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                                        sdbus::ObjectPath("/org/bluez/hci0"))),
      bluez_proxy_(sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                                      sdbus::ObjectPath("/org/bluez"))),
      advertisement_path_(sdbus::ObjectPath("/owo/advertisement1")),
      agent_path_(sdbus::ObjectPath("/owo/agent1")),
      agent_manager_(logger_, *bluez_proxy_),
      agent_(logger_, connection_, agent_path_),
      adapter_(logger_, adapter_proxy_),
      advertising_manager_(logger_, adapter_proxy_),
      advertisement_(logger_, connection_, advertisement_path_),
      gatt_application_(logger_, connection_, adapter_proxy_) {
  logger_.LogInfo("Creating DBusManager");

  if (!agent_manager_.RegisterAgent(agent_path_, "NoInputNoOutput")) {
    logger_.LogError("Failed to register agent");
  }

  if (!agent_manager_.RequestDefaultAgent(agent_path_)) {
    logger_.LogError("Failed to request default agent");
  }

  SetupAdapter();
  SetupName();

  connection_->enterEventLoopAsync();
  if (!gatt_application_.Register()) {
    throw std::runtime_error("Failed to register the BLE GATT application");
  }
} catch (std::exception& e) {
  auto format = std::format("Failed to create DBusManager: {}", e.what());

  logger.LogError(format);

  throw;
}

DbusManager::~DbusManager() {
  gatt_application_.Unregister();
  if (advertising_manager_.IsRegisteredOrPending()) {
    (void) advertising_manager_.UnregisterAdvertisement(advertisement_path_);
  }
  connection_->leaveEventLoop();
}

GattApplication& DbusManager::Gatt() noexcept {
  return gatt_application_;
}

void DbusManager::SetupName() {
  const auto kAdapterAlias = adapter_.get_alias();

  if (!kAdapterAlias) {
    logger_.LogError("Failed to get alias name");
    return;
  }

  const auto kCurrentAlias = kAdapterAlias.value();

  if (kCurrentAlias == alias_) {
    return;
  }

  const auto kRes = adapter_.set_alias(alias_);

  if (!kRes) {
    logger_.LogError("Failed to set alias");
    return;
  }
}

void DbusManager::MakeConnectable(const bool allow_pairing) {
  connectable_requested_ = true;
  // UUID-filtered LE scans do not require general adapter discoverability. Keeping it disabled
  // also avoids a controller-level conflict between classic discovery and custom LE advertising.
  if (!adapter_.set_discoverable(false)) {
    logger_.LogError("Failed to disable general discoverability");
  }
  if (!adapter_.set_pairable(allow_pairing)) {
    logger_.LogError("Failed to set pairable");
  }
}

void DbusManager::DisableNewConnection() {
  connectable_requested_ = false;
  if (advertising_manager_.IsRegisteredOrPending()) {
    if (!advertising_manager_.UnregisterAdvertisement(advertisement_path_)) {
      logger_.LogError("Failed to unregister the BLE advertisement");
    }
  }
  if (!adapter_.set_discoverable(false)) {
    logger_.LogError("Failed to set discoverable");
  }
  if (!adapter_.set_pairable(false)) {
    logger_.LogError("Failed to set pairable");
  }
}

void DbusManager::Poll() {
  if (!connectable_requested_ || !gatt_application_.IsRegistered() ||
      advertising_manager_.IsRegisteredOrPending() ||
      std::chrono::steady_clock::now() < next_advertisement_attempt_) {
    return;
  }
  next_advertisement_attempt_ = std::chrono::steady_clock::now() + std::chrono::seconds{2};
  if (!advertising_manager_.RegisterAdvertisement(advertisement_path_, {})) {
    logger_.LogError("Failed to register the BLE advertisement");
  }
}

void DbusManager::SetupAdapter() {
  const auto powered = adapter_.get_powered();
  if (!powered) {
    logger_.LogError("Failed to read Bluetooth adapter power state");
  } else if (!*powered) {
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
