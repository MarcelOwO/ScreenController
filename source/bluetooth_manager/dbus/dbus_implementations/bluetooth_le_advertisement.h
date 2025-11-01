//
// Created by marce on 5/3/2025.
//

#ifndef BLUETOOTH_LE_ADVERTISEMENT_H
#define BLUETOOTH_LE_ADVERTISEMENT_H
#include "logging/logger.h"
#include "sdbus-c++/IObject.h"

namespace screen_controller {
class BluetoothLEAdvertisement {
 public:
  explicit BluetoothLEAdvertisement(
      const std::shared_ptr<Logger>& logger,
      const std::shared_ptr<sdbus::IConnection>& connection,
      const sdbus::ObjectPath& path);

  ~BluetoothLEAdvertisement() = default;

  // void init() const;

  BluetoothLEAdvertisement(const BluetoothLEAdvertisement&) = delete;
  BluetoothLEAdvertisement(BluetoothLEAdvertisement&&) = delete;
  BluetoothLEAdvertisement& operator=(const BluetoothLEAdvertisement&) = delete;
  BluetoothLEAdvertisement& operator=(BluetoothLEAdvertisement&&) = delete;

 private:
  std::shared_ptr<Logger> logger_;

  sdbus::InterfaceName le_advertisement_interface_name_;
  std::unique_ptr<sdbus::IObject> le_advertisement_;
};
}  // namespace screen_controller

#endif  // BLUETOOTH_LE_ADVERTISEMENT_H
