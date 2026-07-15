//
// Created by marce on 5/3/2025.
//
#pragma once

#include <helper/define.hpp>
#include <logging/logger.hpp>

#include <memory>
#include <string>
#include <string_view>

#include "helper_templates.hpp"
#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::dbus {

class BluetoothAdapter {
public:
  explicit BluetoothAdapter(ILogger& logger, const std::shared_ptr<sdbus::IProxy>& adapter_proxy);

  ~BluetoothAdapter() = default;

  BluetoothAdapter(const BluetoothAdapter&) = delete;
  BluetoothAdapter& operator=(const BluetoothAdapter&) = delete;
  BluetoothAdapter(BluetoothAdapter&&) = delete;
  BluetoothAdapter& operator=(BluetoothAdapter&&) = delete;

  // methods

  [[nodiscard]] Result<void> start_discovery() const;
  [[nodiscard]] Result<void> stop_discovery() const;

  [[nodiscard]] Result<void> remove_device(const sdbus::ObjectPath& device) const;

  [[nodiscard]] Result<void> set_discovery_filter(
      const std::unordered_map<std::string, sdbus::Variant>& filter) const;

  [[nodiscard]] Result<std::vector<std::string>> get_discovery_filters() const;

  [[nodiscard]] Result<sdbus::ObjectPath> connect_device(
      const std::unordered_map<std::string, sdbus::Variant>& properties) const;

  // properties

  [[nodiscard]] Result<std::string_view> get_address() const;

  [[nodiscard]] Result<void> set_alias(std::string_view alias) const;
  [[nodiscard]] Result<std::string> get_alias() const;

  [[nodiscard]] Result<bool> get_powered() const;
  [[nodiscard]] Result<void> set_powered(bool powered);

  [[nodiscard]] Result<bool> get_discoverable() const;
  [[nodiscard]] Result<void> set_discoverable(bool discoverable);

  [[nodiscard]] Result<bool> get_pairable() const;
  [[nodiscard]] Result<void> set_pairable(bool pairable);

  [[nodiscard]] Result<void> set_pairable_timeout(uint32_t timeout);
  [[nodiscard]] Result<uint32_t> get_pairable_timeout() const;
  [[nodiscard]] Result<void> set_discoverable_timeout(uint32_t timeout);
  [[nodiscard]] Result<uint32_t> get_discoverable_timeout() const;

  [[nodiscard]] Result<bool> get_discovering() const;

private:
  ILogger& logger_;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  sdbus::InterfaceName adapter_interface_name_;

  template <typename T>
  Result<T> get(std::string_view name) const {
    return run_dbus_op([&]() -> Result<T> {
      return adapter_proxy_->getProperty(name).onInterface(adapter_interface_name_).get<T>();
    });
  }

  template <typename T>
  Result<void> set(std::string_view name, const T val) {
    return run_dbus_op([&]() -> Result<void> {
      adapter_proxy_->setProperty(name).onInterface(adapter_interface_name_).toValue(val);
      return {};
    });
  }
};

}  // namespace screen_controller::dbus
