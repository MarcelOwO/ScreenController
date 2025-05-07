//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_ADAPTER_H
#define BLUETOOTH_ADAPTER_H
#include <memory>

#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/Types.h"

namespace sdbus {
class IProxy;
}
namespace screen_controller::dbus {

class BluetoothAdapter {
 public:
  BluetoothAdapter(const std::shared_ptr<sdbus::IProxy>& adapter_proxy);
  ~BluetoothAdapter() = default;

  BluetoothAdapter(const BluetoothAdapter&) = delete;
  BluetoothAdapter& operator=(const BluetoothAdapter&) = delete;
  BluetoothAdapter(BluetoothAdapter&&) = delete;
  BluetoothAdapter& operator=(BluetoothAdapter&&) = delete;

  bool StartDiscovery() const;
  bool StopDiscovery() const;

  bool RemoveDevice(const sdbus::ObjectPath& device) const;
  bool SetDiscovery(
      std::unordered_map<std::string, sdbus::Variant> filter) const;
  std::optional<std::vector<std::string>> GetDiscoveryFilters() const;
  std::optional<sdbus::ObjectPath> ConnectDevice(
      std::unordered_map<std::string, sdbus::Variant> properties);

  std::optional<std::string_view> GetAddress();
  std::string_view GettAddressType();
  std::string_view GetName();
  std::string_view GetAlias();

  bool SetAlias(std::string_view alias);
  uint32_t GetClass();

  bool GetConnectable();
  void SetConnectable(bool connectable);

  bool GetPowered();
  bool SetPowered(bool powered) const;
  std::string_view GetPowerState();

  bool GetDiscoverable();
  bool SetDiscoverable(bool discoverable) const;

  bool GetPairable();
  bool SetPairable(bool pairable) const;

  uint32_t GetPairableTimeout();
  void SetPairableTimeout(uint32_t timeout);

  uint32_t GetDiscoverableTimeout();
  void SetDiscoverableTimeout(uint32_t timeout);

  bool GetDiscovering();
  std::vector<std::string> GetUuids();
  std::string_view GetModalias();
  std::vector<std::string> GetRoles();
  std::vector<std::string> GetExperimentalFeatures();
  uint16_t GetManufacturer();
  uint8_t GetVersion();

 private:
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  sdbus::InterfaceName adapter_interface_name_;
};

}  // namespace screen_controller::dbus

#endif  // BLUETOOTH_ADAPTER_H
