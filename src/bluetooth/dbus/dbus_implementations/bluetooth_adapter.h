//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_ADAPTER_H
#define BLUETOOTH_ADAPTER_H

#include <logging/logger.h>

#include <expected>
#include <memory>
#include <string_view>

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

  [[nodiscard]] std::expected<void, std::error_code> start_discovery() const;

  [[nodiscard]] std::expected<void, std::error_code> stop_discovery() const;

  [[nodiscard]] std::expected<void, std::error_code> remove_device(
      const sdbus::ObjectPath& device) const;

  [[nodiscard]] std::expected<void, std::error_code> set_discovery(
      const std::unordered_map<std::string, sdbus::Variant>& filter) const;
  [[nodiscard]] std::optional<std::vector<std::string>> get_discovery_filters()
      const;
  [[nodiscard]] std::optional<sdbus::ObjectPath> connect_device(
      const std::unordered_map<std::string, sdbus::Variant>& properties) const;

  [[nodiscard]] std::optional<std::string_view> get_address() const;

  [[nodiscard]] std::expected<void, std::error_code> set_alias(
      std::string_view alias) const;

  [[nodiscard]] std::expected<std::string_view, std::error_code> get_alias()
      const;

  [[nodiscard]] bool get_powered() const;
  [[nodiscard]] bool set_powered(bool powered) const;

  [[nodiscard]] bool get_discoverable() const;
  [[nodiscard]] bool set_discoverable(bool discoverable) const;

  bool get_pairable();
  [[nodiscard]] bool set_pairable(bool pairable) const;

 private:
  ILogger& logger_;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  sdbus::InterfaceName adapter_interface_name_;
};

}  // namespace screen_controller

#endif  // BLUETOOTH_ADAPTER_H
