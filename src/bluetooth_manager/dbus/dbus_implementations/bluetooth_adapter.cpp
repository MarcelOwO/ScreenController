//
// Created by marce on 5/3/2025.
//

#include "bluetooth_adapter.h"

#include <iostream>

#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller {
BluetoothAdapter::BluetoothAdapter(
    const std::shared_ptr<Logger>& logger,
    const std::shared_ptr<sdbus::IProxy>& adapter_proxy)
    : logger_(logger),
      adapter_proxy_(adapter_proxy),
      adapter_interface_name_(sdbus::InterfaceName("org.bluez.Adapter1")) {
  logger_->LogInfo("Creating bluetooth adapter");
  if (!set_alias("ScreenControllerApp")) {
    logger_->LogError("Failed to set alias");
  }
  if (!get_powered()) {
    if (!set_powered(true)) {
      logger_->LogError("Failed to set powered");
    }
  }

  if (!get_discoverable()) {
    if (!set_discoverable(true)) {
      logger_->LogError("Failed to set discoverable");
    }
  }
  if (!get_pairable()) {
    if (!set_pairable(true)) {
      logger_->LogError("Failed to set pairable");
    }
  }
}

std::expected<void, std::error_code> BluetoothAdapter::start_discovery() const {
  try {
    (void)adapter_proxy_->callMethodAsync("StartDiscovery")
        .onInterface(adapter_interface_name_);
    return {};
  } catch (sdbus::Error& e) {
    logger_->LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothAdapter::stop_discovery() const {
  try {
    (void)adapter_proxy_->callMethod("StopDiscovery")
        .onInterface(adapter_interface_name_);
    return {};
  } catch (sdbus::Error& e) {
    logger_->LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothAdapter::remove_device(
    const sdbus::ObjectPath& device) const {
  try {
    (void)adapter_proxy_->callMethod("RemoveDevice")
        .onInterface(adapter_interface_name_)
        .withArguments(device);
    return {};
  } catch (sdbus::Error& e) {
    logger_->LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothAdapter::set_discovery(
    const std::unordered_map<std::string, sdbus::Variant>& filter) const {
  try {
    (void)adapter_proxy_->callMethod("SetDiscovery")
        .onInterface(adapter_interface_name_)
        .withArguments(filter);
    return {};
  } catch (sdbus::Error& e) {
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::optional<std::vector<std::string>>
BluetoothAdapter::get_discovery_filters() const {
  try {
    std::vector<std::string> filters{};

    adapter_proxy_->callMethod("DiscoveryFilters")
        .onInterface(adapter_interface_name_)
        .storeResultsTo(filters);

    return filters;
  } catch (sdbus::Error& e) {
    std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
    return std::nullopt;
  }
}

std::optional<sdbus::ObjectPath> BluetoothAdapter::connect_device(
    const std::unordered_map<std::string, sdbus::Variant>& properties) const {
  try {
    sdbus::ObjectPath device;

    adapter_proxy_->callMethod("DiscoveryFilters")
        .onInterface(adapter_interface_name_)
        .storeResultsTo(device);

    return device;
  } catch (sdbus::Error& e) {
    std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
    return std::nullopt;
  }
}

std::optional<std::string_view> BluetoothAdapter::get_address() const {
  try {
    return adapter_proxy_->getProperty("Address")
        .onInterface(adapter_interface_name_)
        .get<std::string>();
  } catch (sdbus::Error& e) {
    std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
    return std::nullopt;
  }
}

std::expected<std::string_view, std::error_code> BluetoothAdapter::get_alias()
    const {
  try {
    return adapter_proxy_->getProperty("Alias")
        .onInterface(adapter_interface_name_)
        .get<std::string>();
  } catch (const sdbus::Error& e) {
    logger_->LogError(e.what());
    return std::unexpected(std::error_code(
        std::make_error_code(std::errc::operation_not_permitted)));
  }
}

std::expected<void, std::error_code> BluetoothAdapter::set_alias(
    const std::string_view alias) const {
  try {
    adapter_proxy_->setProperty("Alias")
        .onInterface(adapter_interface_name_)
        .toValue(std::string(alias));
    return {};
  } catch (sdbus::Error& e) {
    logger_->LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

bool BluetoothAdapter::get_powered() const {
  try {
    return adapter_proxy_->getProperty("Powered")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error& e) {
    return false;
  }
}

bool BluetoothAdapter::set_powered(const bool powered) const {
  try {
    adapter_proxy_->setProperty("Powered")
        .onInterface(adapter_interface_name_)
        .toValue(powered);
    return true;
  } catch (const sdbus::Error& e) {
    return false;
  }
}

bool BluetoothAdapter::get_discoverable() const {
  try {
    return adapter_proxy_->getProperty("Discoverable")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error& e) {
    logger_->LogError(e.what());
    return false;
  }
}

bool BluetoothAdapter::set_discoverable(const bool discoverable) const {
  try {
    adapter_proxy_->setProperty("Discoverable")
        .onInterface(adapter_interface_name_)
        .toValue(discoverable);
    return true;
  } catch (const sdbus::Error& e) {
    logger_->LogError(e.what());
    return false;
  }
}
bool BluetoothAdapter::get_pairable() {
  try {
    return adapter_proxy_->getProperty("Pairable")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error& e) {
    logger_->LogError(e.what());
    return false;
  }
}

bool BluetoothAdapter::set_pairable(const bool pairable) const {
  try {
    adapter_proxy_->setProperty("Pairable")
        .onInterface(adapter_interface_name_)
        .toValue(pairable);
    return true;
  } catch (const sdbus::Error& e) {
    logger_->LogError(e.what());
    return false;
  }
}

}  // namespace screen_controller
