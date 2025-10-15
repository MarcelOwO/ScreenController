//
// Created by marce on 5/2/2025.
//

#ifndef BLUETOOTH_DEVICE_H
#define BLUETOOTH_DEVICE_H

#include <sdbus-c++/sdbus-c++.h>
namespace screen_controller::bluetooth::dbus {
class BluetoothDevice {
 public:
  BluetoothDevice(std::shared_ptr<sdbus::IConnection> connection,
                  const sdbus::ObjectPath& device);
  ~BluetoothDevice() = default;

  BluetoothDevice(const BluetoothDevice&) = delete;
  BluetoothDevice& operator=(const BluetoothDevice&) = delete;
  BluetoothDevice(BluetoothDevice&&) = delete;
  BluetoothDevice& operator=(BluetoothDevice&&) = delete;

  bool Connect() const;
  bool Disconnect() const;
  bool ConnectProfile(std::string_view uuid) const;
  bool DisconnectProfile(std::string_view uuid) const;
  bool Pair() const;
  bool CancelPairing() const;
  std::optional<std::vector<std::vector<uint8_t>>> GetServiceRecords();

  std::optional<std::string_view> GetAddress() const;
  std::optional<std::string_view> GetAddressType() const;
  std::optional<std::string_view> GetName() const;
  std::optional<std::string_view> GetIcon() const;

  uint32_t GetClass() const;
  uint16_t GetAppearance();
  std::vector<std::string> GetUUIDs();
  bool GetPaired();
  bool GetBonded();
  bool GetConnected();
  bool GetTrusted();
  void SetTrusted(bool trusted);
  bool GetBlocked();
  void SetBlocked(bool blocked);

  bool GetWakeAllowed();
  void SetWakeAllowed(bool allowed);

  std::string GetAlias();
  void SetAlias(std::string_view alias);

  sdbus::ObjectPath GetAdapter;
  bool GetLegacyPairing();
  bool GetCablePairing();
  std::string GetModalias();
  int16_t GetRSSI();
  int16_t GetTxPower();

  std::unordered_map<uint16_t, std::vector<uint8_t>> GetManufacturerData();
  std::unordered_map<std::string_view, std::vector<uint8_t>> GetServiceData();
  bool GetServicesResolved();

  std::vector<uint8_t> GetAdvertisingFlags();
  std::unordered_map<uint8_t, std::vector<uint8_t>> GetAdvertisingData();
  std::unordered_map<sdbus::ObjectPath, std::vector<uint8_t>> GetSets();

  std::string GetPreferredBearer();
  void SetPreferredBearer(std::string bearer);

 private:
  std::unique_ptr<sdbus::IProxy> device_proxy_;
  sdbus::InterfaceName device_interface_;
};
}  // namespace screen_controller::dbus
#endif  // BLUETOOTH_DEVICE_H
