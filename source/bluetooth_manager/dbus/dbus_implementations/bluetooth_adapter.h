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
namespace screen_controller::bluetooth::dbus {

class BluetoothAdapter {
 public:
  explicit BluetoothAdapter(
      const std::shared_ptr<sdbus::IProxy> &adapter_proxy);
  ~BluetoothAdapter() = default;

  BluetoothAdapter(const BluetoothAdapter &) = delete;
  BluetoothAdapter &operator=(const BluetoothAdapter &) = delete;
  BluetoothAdapter(BluetoothAdapter &&) = delete;
  BluetoothAdapter &operator=(BluetoothAdapter &&) = delete;

  bool init();

  bool start_discovery() const;
  bool stop_discovery() const;

  bool remove_device(const sdbus::ObjectPath &device) const;

  bool set_discovery(
      std::unordered_map<std::string, sdbus::Variant> filter) const;
  std::optional<std::vector<std::string>> get_discovery_filters() const;
  std::optional<sdbus::ObjectPath> connect_device(
      std::unordered_map<std::string, sdbus::Variant> properties) const;

  std::optional<std::string_view> get_address() const;
  std::string_view gett_address_type();
  std::string_view get_name();
  std::string_view get_alias();

  bool set_alias(std::string_view alias) const;
  uint32_t get_class();

  bool get_connectable();
  void set_connectable(bool connectable);

  bool get_powered();
  bool set_powered(bool powered) const;
  std::string_view get_power_state();

  bool get_discoverable();
  bool set_discoverable(bool discoverable) const;

  bool get_pairable();
  bool set_pairable(bool pairable) const;

  uint32_t get_pairable_timeout();
  void set_pairable_timeout(uint32_t timeout);

  uint32_t get_discoverable_timeout();
  void set_discoverable_timeout(uint32_t timeout);

  bool get_discovering();
  std::vector<std::string> get_uuids();
  std::string_view get_modalias();
  std::vector<std::string> get_roles();
  std::vector<std::string> get_experimental_features();
  uint16_t get_manufacturer();
  uint8_t get_version();

 private:
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  sdbus::InterfaceName adapter_interface_name_;
};

}  // namespace screen_controller::bluetooth::dbus

#endif  // BLUETOOTH_ADAPTER_H
