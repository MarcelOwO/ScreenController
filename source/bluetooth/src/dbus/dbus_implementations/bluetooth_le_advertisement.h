//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_LE_ADVERTISEMENT_H
#define BLUETOOTH_LE_ADVERTISEMENT_H
#include "logging/logger.h"
#include "sdbus-c++/IObject.h"

<<<<<<<< HEAD:source/bluetooth_manager/dbus/dbus_implementations/bluetooth_le_advertisement.h
namespace screen_controller {
class BluetoothLEAdvertisement {
 public:
  explicit BluetoothLEAdvertisement(
      const std::shared_ptr<Logger>& logger,
      const std::shared_ptr<sdbus::IConnection>& connection,
      const sdbus::ObjectPath& path);
========
namespace screen_controller::dbus {
class BluetoothLEAdvertisement {
public:
  explicit BluetoothLEAdvertisement(ILogger& logger,
                                    const std::shared_ptr<sdbus::IConnection>& connection,
                                    const sdbus::ObjectPath& path);
>>>>>>>> origin/dev:source/bluetooth/src/dbus/dbus_implementations/bluetooth_le_advertisement.h

  ~BluetoothLEAdvertisement() = default;

  // void init() const;

  BluetoothLEAdvertisement(const BluetoothLEAdvertisement&) = delete;
  BluetoothLEAdvertisement(BluetoothLEAdvertisement&&) = delete;
  BluetoothLEAdvertisement& operator=(const BluetoothLEAdvertisement&) = delete;
  BluetoothLEAdvertisement& operator=(BluetoothLEAdvertisement&&) = delete;

<<<<<<<< HEAD:source/bluetooth_manager/dbus/dbus_implementations/bluetooth_le_advertisement.h
 private:
  std::shared_ptr<Logger> logger_;
========
private:
  ILogger& logger_;
>>>>>>>> origin/dev:source/bluetooth/src/dbus/dbus_implementations/bluetooth_le_advertisement.h

  sdbus::InterfaceName le_advertisement_interface_name_;
  std::unique_ptr<sdbus::IObject> le_advertisement_;
};
<<<<<<<< HEAD:source/bluetooth_manager/dbus/dbus_implementations/bluetooth_le_advertisement.h
}  // namespace screen_controller
========
}  // namespace screen_controller::dbus
>>>>>>>> origin/dev:source/bluetooth/src/dbus/dbus_implementations/bluetooth_le_advertisement.h

#endif  // BLUETOOTH_LE_ADVERTISEMENT_H
