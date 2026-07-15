//
// Created by marce on 5/3/2025.
//

#include "bluetooth_adapter.hpp"

#include <helper/define.hpp>

#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Message.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::dbus {

BluetoothAdapter::BluetoothAdapter(ILogger& logger,
                                   const std::shared_ptr<sdbus::IProxy>& adapter_proxy)
    : logger_(logger),
      adapter_proxy_(adapter_proxy),
      adapter_interface_name_(sdbus::InterfaceName("org.bluez.Adapter1")) {
  logger_.LogInfo("Creating bluetooth adapter");
}

Result<void> BluetoothAdapter::set_alias(const std::string_view alias) const {
  return run_dbus_op([&]() -> Result<void> {
    adapter_proxy_->setProperty("Alias")
        .onInterface(adapter_interface_name_)
        .toValue(std::string(alias));
    return {};
  });
}

Result<std::string> BluetoothAdapter::get_alias() const {
  return run_dbus_op([&]() -> Result<std::string> {
    return adapter_proxy_->getProperty("Alias")
        .onInterface(adapter_interface_name_)
        .get<std::string>();
  });
}

Result<bool> BluetoothAdapter::get_powered() const {
  try {
    return adapter_proxy_->getProperty("Powered").onInterface(adapter_interface_name_).get<bool>();
  } catch (const sdbus::Error& e) {
    return false;
  }
}

Result<void> BluetoothAdapter::set_powered(const bool powered) {
  return run_dbus_op([&]() -> Result<void> {
    adapter_proxy_->setProperty("Powered").onInterface(adapter_interface_name_).toValue(powered);
    return {};
  });
}

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

}  // namespace screen_controller::dbus
