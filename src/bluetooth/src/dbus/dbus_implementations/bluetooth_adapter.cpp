//
// Created by marce on 5/3/2025.
//

#include "bluetooth_adapter.h"

#include <define.h>

#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Message.h"
#include "sdbus-c++/Types.h"

namespace screen_controller {

// finish reworking all of this ... someday xD

BluetoothAdapter::BluetoothAdapter(
    ILogger& logger, const std::shared_ptr<sdbus::IProxy>& adapter_proxy)
    : logger_(logger),
      adapter_proxy_(adapter_proxy),
      adapter_interface_name_(sdbus::InterfaceName("org.bluez.Adapter1")) {
  logger_.LogInfo("Creating bluetooth adapter");
}

Result<void> BluetoothAdapter::start_discovery() const {
  return run_dbus_op([&]() -> Result<void> {
    adapter_proxy_->callMethodAsync("StartDiscovery")
        .onInterface(adapter_interface_name_);
    return {};
  });
}

Result<void> BluetoothAdapter::stop_discovery() const {
  return run_dbus_op([&]() -> Result<void> {
    adapter_proxy_->callMethod("StopDiscovery")
        .onInterface(adapter_interface_name_);
    return {};
  });
}

Result<void> BluetoothAdapter::remove_device(
    const sdbus::ObjectPath& device) const {
  return run_dbus_op([&]() -> Result<void> {
    (void)adapter_proxy_->callMethod("RemoveDevice")
        .onInterface(adapter_interface_name_)
        .withArguments(device);
    return {};
  });
}

Result<void> BluetoothAdapter::set_discovery_filter(
    const std::unordered_map<std::string, sdbus::Variant>& filter) const {
  return run_dbus_op([&]() -> Result<void> {
    (void)adapter_proxy_->callMethod("SetDiscovery")
        .onInterface(adapter_interface_name_)
        .withArguments(filter);
    return {};
  });
}

Result<std::vector<std::string>> BluetoothAdapter::get_discovery_filters()
    const {
  return run_dbus_op([&] -> Result<std::vector<std::string>> {
    std::vector<std::string> filters{};

    adapter_proxy_->callMethod("DiscoveryFilters")
        .onInterface(adapter_interface_name_)
        .storeResultsTo(filters);

    return filters;
  });
}

Result<sdbus::ObjectPath> BluetoothAdapter::connect_device(
    const std::unordered_map<std::string, sdbus::Variant>& properties) const {
  return run_dbus_op([&]() -> Result<sdbus::ObjectPath> {
    sdbus::ObjectPath device;
    adapter_proxy_->callMethod("DiscoveryFilters")
        .onInterface(adapter_interface_name_)
        .storeResultsTo(device);
    return device;
  });
}

Result<std::string_view> BluetoothAdapter::get_address() const {
  return run_dbus_op([&]() -> Result<std::string_view> {
    return adapter_proxy_->getProperty("Address")
        .onInterface(adapter_interface_name_)
        .get<std::string>();
  });
}

Result<std::string_view> BluetoothAdapter::get_address_type() const {
  logger_.LogError(
      "BluetoothAdapter::get_address_type has not been implemented");
  return std::string_view();
}

Result<std::string_view> BluetoothAdapter::get_name() {
  logger_.LogError("BluetoothAdapter::get_name has not been implemented");
  return std::string_view();
}

Result<void> BluetoothAdapter::set_alias(const std::string_view alias) const {
  return run_dbus_op([&]() -> Result<void> {
    adapter_proxy_->setProperty("Alias")
        .onInterface(adapter_interface_name_)
        .toValue(std::string(alias));
    return {};
  });
}

Result<std::string_view> BluetoothAdapter::get_alias() const {
  return run_dbus_op([&]() -> Result<std::string_view> {
    return adapter_proxy_->getProperty("Alias")
        .onInterface(adapter_interface_name_)
        .get<std::string>();
  });
}

Result<uint32_t> BluetoothAdapter::get_class() const {
  logger_.LogError("BluetoothAdapter::get_class has not been implemented");
  return {};
}

Result<bool> BluetoothAdapter::get_connectable() const {}
Result<void> BluetoothAdapter::set_connectable(bool connectable) {}

Result<bool> BluetoothAdapter::get_powered() const {
  try {
    return adapter_proxy_->getProperty("Powered")
        .onInterface(adapter_interface_name_)
        .get<bool>();
  } catch (const sdbus::Error& e) {
    return false;
  }
}

Result<void> BluetoothAdapter::set_powered(const bool powered) {
  return run_dbus_op([&]() -> Result<void> {
    adapter_proxy_->setProperty("Powered")
        .onInterface(adapter_interface_name_)
        .toValue(powered);
    return {};
  });
}

Result<bool> BluetoothAdapter::get_powerstate() const {}

Result<bool> BluetoothAdapter::get_discoverable() const {
  return get<bool>("Discoverable");
}

Result<void> BluetoothAdapter::set_discoverable(const bool discoverable) {
  return set("Discoverable", discoverable);
}

Result<bool> BluetoothAdapter::get_pairable() const {
  return get<bool>("Pairable");
}

Result<void> BluetoothAdapter::set_pairable(const bool pairable) {
  return set("Pairable", pairable);
}

Result<uint32_t> BluetoothAdapter::get_pairable_timeout() const {
  return get<uint32_t>("PairableTimeout");
}

Result<void> BluetoothAdapter::set_pairable_timeout(uint32_t timeout) {
  return set<uint32_t>("PairableTimeout", timeout);
}

Result<uint32_t> BluetoothAdapter::get_discoverable_timeout() const {
  return get<u32>("DiscoverableTimeout");
}
Result<void> BluetoothAdapter::set_discoverable_timeout(uint32_t timeout) {
  return set<u32>("DiscoverableTimeout", timeout);
}

Result<bool> BluetoothAdapter::get_discovering() const {
  return get<bool>("Discovering");
}

Result<std::vector<std::string>> BluetoothAdapter::get_uuids() const {
  return get<std::vector<std::string>>("UUIDs");
}

Optional<std::string> BluetoothAdapter::get_modalias() const {}

Result<std::vector<std::string>> BluetoothAdapter::get_roles() const {}

Result<std::vector<std::string>> BluetoothAdapter::get_experimental_features()
    const {}

Result<uint32_t> BluetoothAdapter::get_manufacturer() const {}

Result<std::byte> BluetoothAdapter::get_version() const {}

}  // namespace screen_controller
