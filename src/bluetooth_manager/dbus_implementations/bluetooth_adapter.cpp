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
    const std::shared_ptr<sdbus::IProxy>& adapter_proxy)
    : adapter_proxy_(adapter_proxy),
      adapter_interface_name_(sdbus::InterfaceName("org.bluez.Adapter1")) {}

bool BluetoothAdapter::init() {
  PCHECK(SetAlias("ScreenControllerApp")) << "Failed to set alias";

  if (!GetPowered()) {
    PCHECK(SetPowered(true)) << "Failed to set powered";
  }
  if (!GetDiscoverable()) {
    PCHECK(SetDiscoverable(true)) << "Failed to set discoverable";
  }
  if (!GetPairable()) {
    PCHECK(SetPairable(true)) << "Failed to set pairable";
  }

  return true;
}

bool BluetoothAdapter::StartDiscovery() const {
  try {
    (void)adapter_proxy_->callMethodAsync("StartDiscovery")
        .onInterface(adapter_interface_name_);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothAdapter::StopDiscovery() const {
  try {
    (void)adapter_proxy_->callMethod("StopDiscovery")
        .onInterface(adapter_interface_name_);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothAdapter::RemoveDevice(const sdbus::ObjectPath& device) const {
  try {
    (void)adapter_proxy_->callMethod("RemoveDevice")
        .onInterface(adapter_interface_name_)
        .withArguments(device);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothAdapter::SetDiscovery(
    std::unordered_map<std::string, sdbus::Variant> filter) const {
  try {
    (void)adapter_proxy_->callMethod("SetDiscovery")
        .onInterface(adapter_interface_name_)
        .withArguments(filter);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

std::optional<std::vector<std::string>> BluetoothAdapter::GetDiscoveryFilters()
    const {
  try {
    std::vector<std::string> filters{};

    adapter_proxy_->callMethod("DiscoveryFilters")
        .onInterface(adapter_interface_name_)
        .storeResultsTo(filters);

    return filters;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<sdbus::ObjectPath> BluetoothAdapter::ConnectDevice(
    std::unordered_map<std::string, sdbus::Variant> properties) {
  try {
    sdbus::ObjectPath device;

    adapter_proxy_->callMethod("DiscoveryFilters")
        .onInterface(adapter_interface_name_)
        .storeResultsTo(device);

    return device;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<std::string_view> BluetoothAdapter::GetAddress() {
  try {
    const auto value = adapter_proxy_->getProperty("Address")
                           .onInterface(adapter_interface_name_)
                           .get<std::string>();
    return value;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::string_view BluetoothAdapter::GettAddressType() {}
std::string_view BluetoothAdapter::GetName() {}
std::string_view BluetoothAdapter::GetAlias() {}
bool BluetoothAdapter::SetAlias(std::string_view alias) {
  try {
    adapter_proxy_->setProperty("Alias")
        .onInterface(adapter_interface_name_)
        .toValue(std::string(alias));
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}
uint32_t BluetoothAdapter::GetClass() {}

bool BluetoothAdapter::GetConnectable() {}
void BluetoothAdapter::SetConnectable(bool connectable) {}

bool BluetoothAdapter::GetPowered() {
  try {
    return adapter_proxy_->getProperty("Powered")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAdapter::SetPowered(const bool powered) const {
  try {
    adapter_proxy_->setProperty("Powered")
        .onInterface(adapter_interface_name_)
        .toValue(powered);
    return true;
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}
std::string_view BluetoothAdapter::GetPowerState() {
  PLOG(FATAL) << "Not implemented";
  return std::string_view();
}

bool BluetoothAdapter::GetDiscoverable() {
  try {
    return adapter_proxy_->getProperty("Discoverable")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAdapter::SetDiscoverable(const bool discoverable) const {
  try {
    adapter_proxy_->setProperty("Discoverable")
        .onInterface(adapter_interface_name_)
        .toValue(discoverable);
    return true;
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}
bool BluetoothAdapter::GetPairable() {
  try {
    return adapter_proxy_->getProperty("Pairable")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

bool BluetoothAdapter::SetPairable(const bool pairable) const {
  try {
    adapter_proxy_->setProperty("Pairable")
        .onInterface(adapter_interface_name_)
        .toValue(pairable);
    return true;
  } catch (const sdbus::Error& e) {
    PLOG(ERROR) << e.what();
    return false;
  }
}

uint32_t BluetoothAdapter::GetPairableTimeout() {}
void BluetoothAdapter::SetPairableTimeout(uint32_t timeout) {}
uint32_t BluetoothAdapter::GetDiscoverableTimeout() {}
void BluetoothAdapter::SetDiscoverableTimeout(uint32_t timeout) {}
bool BluetoothAdapter::GetDiscovering() {}
std::vector<std::string> BluetoothAdapter::GetUuids() {}
std::string_view BluetoothAdapter::GetModalias() {}
std::vector<std::string> BluetoothAdapter::GetRoles() {}
std::vector<std::string> BluetoothAdapter::GetExperimentalFeatures() {}
uint16_t BluetoothAdapter::GetManufacturer() {}
uint8_t BluetoothAdapter::GetVersion() {}
}  // namespace screen_controller::dbus