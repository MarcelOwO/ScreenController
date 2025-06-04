//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_LE_ADVERTISEMENT_H
#define BLUETOOTH_LE_ADVERTISEMENT_H
#include "sdbus-c++/IObject.h"

namespace screen_controller::bluetooth::dbus {
class BluetoothLEAdvertisement {
 public:
  BluetoothLEAdvertisement();
  BluetoothLEAdvertisement(
      const std::shared_ptr<sdbus::IConnection>& connection,
      const sdbus::ObjectPath& path);
  void init() const;
  ~BluetoothLEAdvertisement() = default;

  BluetoothLEAdvertisement(const BluetoothLEAdvertisement&) = delete;
  BluetoothLEAdvertisement(BluetoothLEAdvertisement&&) = delete;
  BluetoothLEAdvertisement& operator=(const BluetoothLEAdvertisement&) = delete;
  BluetoothLEAdvertisement& operator=(BluetoothLEAdvertisement&&) = delete;




 private:
  sdbus::InterfaceName le_advertisement_interface_name_;
  std::unique_ptr<sdbus::IObject> le_advertisement_;
};
}  // namespace screen_controller::dbus

#endif  // BLUETOOTH_LE_ADVERTISEMENT_H
