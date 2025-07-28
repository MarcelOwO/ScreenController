//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_LE_ADVERTISING_MANAGER_H
#define BLUETOOTH_LE_ADVERTISING_MANAGER_H
#include <memory>

#include "sdbus-c++/IProxy.h"

namespace screen_controller::bluetooth::dbus {
class BluetoothLeAdvertisingManager {
public:
  explicit BluetoothLeAdvertisingManager(
      const std::shared_ptr<sdbus::IProxy>& adapter_proxy);
  ~BluetoothLeAdvertisingManager();

  BluetoothLeAdvertisingManager(const BluetoothLeAdvertisingManager&) = delete;
  BluetoothLeAdvertisingManager& operator=(
      const BluetoothLeAdvertisingManager&) = delete;
  BluetoothLeAdvertisingManager(const BluetoothLeAdvertisingManager&&) = delete;
  BluetoothLeAdvertisingManager& operator=(
      const BluetoothLeAdvertisingManager&&) = delete;

  [[nodiscard]] bool RegisterAdvertisement(
      const sdbus::ObjectPath& advertisement,
      const std::unordered_map<std::string, sdbus::Variant>& options) const;

  [[nodiscard]] bool UnregisterAdvertisement() const;
  [[nodiscard]] std::optional<uint8_t> GetActiveInstances() const;
  [[nodiscard]] std::optional<uint8_t> GetSupportedInstances() const;
  [[nodiscard]] std::optional<std::vector<std::string>>
  GetSupportedIncludes() const;
  [[nodiscard]] std::optional<std::vector<std::string>>
  GetSupportedSecondaryChannels() const;
  [[nodiscard]] std::optional<std::unordered_map<std::string, sdbus::Variant>>
  GetSupportedCapabilities() const;
  [[nodiscard]] std::optional<std::vector<std::string>>
  GetSupportedFeatures() const;

private:
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  sdbus::InterfaceName adv_manager_interface_name;
};
} // namespace screen_controller::dbus

#endif  // BLUETOOTH_LE_ADVERTISING_MANAGER_H