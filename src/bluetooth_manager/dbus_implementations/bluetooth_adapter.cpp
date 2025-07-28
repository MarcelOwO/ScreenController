//
// Created by marce on 5/3/2025.
//

#include "bluetooth_adapter.h"

#include <ng-log/logging.h>

#include <iostream>

#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::bluetooth::dbus {
BluetoothAdapter::BluetoothAdapter(
    const std::shared_ptr<sdbus::IProxy> &adapter_proxy)
    : adapter_proxy_(adapter_proxy),
      adapter_interface_name_(sdbus::InterfaceName("org.bluez.Adapter1")) {}

bool BluetoothAdapter::init() {
  PCHECK(set_alias("ScreenControllerApp")) << "Failed to set alias";

  if (!get_powered()) {
    PCHECK(set_powered(true)) << "Failed to set powered";
  }

  if (!get_discoverable()) {
    PCHECK(set_discoverable(true)) << "Failed to set discoverable";
  }
  if (!get_pairable()) {
    PCHECK(set_pairable(true)) << "Failed to set pairable";
  }

  return true;
}

bool BluetoothAdapter::start_discovery() const {
  try {
    (void)adapter_proxy_->callMethodAsync("StartDiscovery")
        .onInterface(adapter_interface_name_);
    return true;
  } catch (sdbus::Error &e) {
    LOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAdapter::stop_discovery() const {
  try {
    (void)adapter_proxy_->callMethod("StopDiscovery")
        .onInterface(adapter_interface_name_);
    return true;
  } catch (sdbus::Error &e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothAdapter::remove_device(const sdbus::ObjectPath &device) const {
  try {
    (void)adapter_proxy_->callMethod("RemoveDevice")
        .onInterface(adapter_interface_name_)
        .withArguments(device);
    return true;
  } catch (sdbus::Error &e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothAdapter::set_discovery(
    std::unordered_map<std::string, sdbus::Variant> filter) const {
  try {
    (void)adapter_proxy_->callMethod("SetDiscovery")
        .onInterface(adapter_interface_name_)
        .withArguments(filter);
    return true;
  } catch (sdbus::Error &e) {
    std::cerr << e.what() << std::endl;
    return false;
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
  } catch (sdbus::Error &e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<sdbus::ObjectPath> BluetoothAdapter::connect_device(
    std::unordered_map<std::string, sdbus::Variant> properties) const {
  try {
    sdbus::ObjectPath device;

    adapter_proxy_->callMethod("DiscoveryFilters")
        .onInterface(adapter_interface_name_)
        .storeResultsTo(device);

    return device;
  } catch (sdbus::Error &e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<std::string_view> BluetoothAdapter::get_address() const {
  try {
    const auto value = adapter_proxy_->getProperty("Address")
                           .onInterface(adapter_interface_name_)
                           .get<std::string>();
    return value;
  } catch (sdbus::Error &e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::string_view BluetoothAdapter::gett_address_type() {
  CHECK(false) << "Not implemented";
}
std::string_view BluetoothAdapter::get_name() {
  CHECK(false) << "Not implemented";
}
std::string_view BluetoothAdapter::get_alias() {
  CHECK(false) << "Not implemented";
}
bool BluetoothAdapter::set_alias(const std::string_view alias) const {
  try {
    adapter_proxy_->setProperty("Alias")
        .onInterface(adapter_interface_name_)
        .toValue(std::string(alias));
    return true;
  } catch (sdbus::Error &e) {
    LOG(ERROR) << e.what();
    return false;
  }
}
uint32_t BluetoothAdapter::get_class() { CHECK(false) << "Not implemented"; }

bool BluetoothAdapter::get_connectable() { CHECK(false) << "Not implemented"; }
void BluetoothAdapter::set_connectable(bool connectable) {
  CHECK(false) << "Not implemented";
}

bool BluetoothAdapter::get_powered() {
  try {
    return adapter_proxy_->getProperty("Powered")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error &e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAdapter::set_powered(const bool powered) const {
  try {
    adapter_proxy_->setProperty("Powered")
        .onInterface(adapter_interface_name_)
        .toValue(powered);
    return true;
  } catch (const sdbus::Error &e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}
std::string_view BluetoothAdapter::get_power_state() {
  CHECK(false) << "Not implemented";
}

bool BluetoothAdapter::get_discoverable() {
  try {
    return adapter_proxy_->getProperty("Discoverable")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error &e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAdapter::set_discoverable(const bool discoverable) const {
  try {
    adapter_proxy_->setProperty("Discoverable")
        .onInterface(adapter_interface_name_)
        .toValue(discoverable);
    return true;
  } catch (const sdbus::Error &e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}
bool BluetoothAdapter::get_pairable() {
  try {
    return adapter_proxy_->getProperty("Pairable")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error &e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAdapter::set_pairable(const bool pairable) const {
  try {
    adapter_proxy_->setProperty("Pairable")
        .onInterface(adapter_interface_name_)
        .toValue(pairable);
    return true;
  } catch (const sdbus::Error &e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

uint32_t BluetoothAdapter::get_pairable_timeout() {
  CHECK(false) << "Not implemented";
}
void BluetoothAdapter::set_pairable_timeout(uint32_t timeout) {
  CHECK(false) << "Not implemented";
}
uint32_t BluetoothAdapter::get_discoverable_timeout() {
  CHECK(false) << "Not implemented";
}
void BluetoothAdapter::set_discoverable_timeout(uint32_t timeout) {
  CHECK(false) << "Not implemented";
}
bool BluetoothAdapter::get_discovering() { CHECK(false) << "Not implemented"; }
std::vector<std::string> BluetoothAdapter::get_uuids() {
  CHECK(false) << "Not implemented";
}
std::string_view BluetoothAdapter::get_modalias() {
  CHECK(false) << "Not implemented";
}
std::vector<std::string> BluetoothAdapter::get_roles() {
  CHECK(false) << "Not implemented";
}
std::vector<std::string> BluetoothAdapter::get_experimental_features() {
  CHECK(false) << "Not implemented";
}
uint16_t BluetoothAdapter::get_manufacturer() {
  CHECK(false) << "Not implemented";
}
uint8_t BluetoothAdapter::get_version() { CHECK(false) << "Not implemented"; }
}  // namespace screen_controller::bluetooth::dbus