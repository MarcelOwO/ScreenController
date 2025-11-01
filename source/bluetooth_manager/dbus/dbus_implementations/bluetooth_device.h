//
// Created by marce on 5/2/2025.
//

#ifndef BLUETOOTH_DEVICE_H
#define BLUETOOTH_DEVICE_H

#include <logging/logger.h>
#include <sdbus-c++/sdbus-c++.h>

#include <expected>

namespace screen_controller {
class BluetoothDevice {
 public:
  explicit BluetoothDevice(
      const std::shared_ptr<Logger>& logger,
      const std::shared_ptr<sdbus::IConnection>& connection,
      const sdbus::ObjectPath& device);

  ~BluetoothDevice() = default;

  BluetoothDevice(const BluetoothDevice&) = delete;
  BluetoothDevice& operator=(const BluetoothDevice&) = delete;
  BluetoothDevice(BluetoothDevice&&) = delete;
  BluetoothDevice& operator=(BluetoothDevice&&) = delete;

  [[nodiscard]] std::expected<void, std::error_code> Connect() const;
  [[nodiscard]] std::expected<void, std::error_code> Disconnect() const;
  [[nodiscard]] std::expected<void, std::error_code> ConnectProfile(
      std::string_view uuid) const;
  [[nodiscard]] std::expected<void, std::error_code> DisconnectProfile(
      std::string_view uuid) const;
  [[nodiscard]] std::expected<void, std::error_code> Pair() const;
  [[nodiscard]] std::expected<void, std::error_code> CancelPairing() const;
  [[nodiscard]] std::optional<std::vector<std::vector<uint8_t>>>
  GetServiceRecords() const;

  [[nodiscard]] std::optional<std::string_view> GetAddress() const;
  [[nodiscard]] std::optional<std::string_view> GetAddressType() const;
  [[nodiscard]] std::optional<std::string> GetName() const;
  [[nodiscard]] std::optional<std::string_view> GetIcon() const;

  [[nodiscard]] uint32_t GetClass() const;
  [[nodiscard]] uint16_t GetAppearance() const;

  // sdbus::ObjectPath GetAdapter;
 private:
  std::shared_ptr<Logger> logger_;
  std::unique_ptr<sdbus::IProxy> device_proxy_;
  sdbus::InterfaceName device_interface_;
};
}  // namespace screen_controller
#endif  // BLUETOOTH_DEVICE_H
