//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_ADAPTER_H
#define BLUETOOTH_ADAPTER_H

#include <define.h>
#include <logging/logger.h>

#include <memory>
#include <string_view>

#include "helper_templates.h"
#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller {

class BluetoothAdapter {
 public:
  explicit BluetoothAdapter(
      ILogger& logger, const std::shared_ptr<sdbus::IProxy>& adapter_proxy);

  ~BluetoothAdapter() = default;

  BluetoothAdapter(const BluetoothAdapter&) = delete;
  BluetoothAdapter& operator=(const BluetoothAdapter&) = delete;
  BluetoothAdapter(BluetoothAdapter&&) = delete;
  BluetoothAdapter& operator=(BluetoothAdapter&&) = delete;

  // methods

  [[nodiscard]] Result<void> start_discovery() const;
  [[nodiscard]] Result<void> stop_discovery() const;

  [[nodiscard]] Result<void> remove_device(
      const sdbus::ObjectPath& device) const;

  [[nodiscard]] Result<void> set_discovery_filter(
      const std::unordered_map<std::string, sdbus::Variant>& filter) const;

  [[nodiscard]] Result<std::vector<std::string>> get_discovery_filters() const;

  [[nodiscard]] Result<sdbus::ObjectPath> connect_device(
      const std::unordered_map<std::string, sdbus::Variant>& properties) const;

  // properties

  [[nodiscard]] Result<std::string_view> get_address() const;

  [[nodiscard]] Result<std::string_view> get_address_type() const;

  [[nodiscard]] Result<std::string_view> get_name();

  [[nodiscard]] Result<void> set_alias(std::string_view alias) const;
  [[nodiscard]] Result<std::string_view> get_alias() const;

  [[nodiscard]] Result<uint32_t> get_class() const;

  [[nodiscard]] Result<bool> get_connectable() const;
  [[nodiscard]] Result<void> set_connectable(bool connectable);

  [[nodiscard]] Result<bool> get_powered() const;
  [[nodiscard]] Result<void> set_powered(bool powered);

  [[nodiscard]] Result<bool> get_powerstate() const;

  [[nodiscard]] Result<bool> get_discoverable() const;
  [[nodiscard]] Result<void> set_discoverable(bool discoverable);

  [[nodiscard]] Result<bool> get_pairable() const;
  [[nodiscard]] Result<void> set_pairable(bool pairable);

  /*
    uint32 PairableTimeout [readwrite] (Default: 0)

    The pairable timeout in seconds. A value of zero means that the timeout is
    disabled and it will stay in pairable mode forever.
  */
  [[nodiscard]] Result<uint32_t> get_pairable_timeout() const;

  /*
    uint32 PairableTimeout [readwrite] (Default: 0)

    The pairable timeout in seconds. A value of zero means that the timeout is
    disabled and it will stay in pairable mode forever.
  */
  [[nodiscard]] Result<void> set_pairable_timeout(uint32_t timeout);

  /*
    uint32 DiscoverableTimeout [readwrite] (Default: 180)

    The discoverable timeout in seconds. A value of zero means that the timeout
    is disabled and it will stay in discoverable/limited mode forever.
  */
  [[nodiscard]] Result<uint32_t> get_discoverable_timeout() const;

  /*
    uint32 DiscoverableTimeout [readwrite] (Default: 180)

    The discoverable timeout in seconds. A value of zero means that the timeout
    is disabled and it will stay in discoverable/limited mode forever.
  */
  [[nodiscard]] Result<void> set_discoverable_timeout(uint32_t timeout);

  [[nodiscard]] Result<bool> get_discovering() const;

  [[nodiscard]] Result<std::vector<std::string>> get_uuids() const;

  [[nodiscard]] Optional<std::string> get_modalias() const;

  [[nodiscard]] Result<std::vector<std::string>> get_roles() const;

  [[nodiscard]] Result<std::vector<std::string>> get_experimental_features()
      const;

  [[nodiscard]] Result<uint32_t> get_manufacturer() const;

  [[nodiscard]] Result<std::byte> get_version() const;

 private:
  ILogger& logger_;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  sdbus::InterfaceName adapter_interface_name_;

  template <typename T>
  Result<T> get(std::string_view name) const {
    return run_dbus_op([&]() -> Result<T> {
      return adapter_proxy_->getProperty(name)
          .onInterface(adapter_interface_name_)
          .get<T>();
    });
  }

  template <typename T>
  Result<void> set(std::string_view name, const T val) {
    return run_dbus_op([&]() -> Result<void> {
      adapter_proxy_->setProperty(name)
          .onInterface(adapter_interface_name_)
          .toValue(val);
      return {};
    });
  }
};

}  // namespace screen_controller

#endif  // BLUETOOTH_ADAPTER_H
