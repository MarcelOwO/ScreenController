//
// Created by marce on 5/3/2025.
//

#pragma once

#include <logging/logger.hpp>
#include "sdbus-c++/IObject.h"

namespace screen_controller::dbus {
class BluetoothLEAdvertisement {
public:
  explicit BluetoothLEAdvertisement(ILogger& logger,
                                    const std::shared_ptr<sdbus::IConnection>& connection,
                                    const sdbus::ObjectPath& path);

  ~BluetoothLEAdvertisement() = default;

  // void init() const;

  BluetoothLEAdvertisement(const BluetoothLEAdvertisement&) = delete;
  BluetoothLEAdvertisement(BluetoothLEAdvertisement&&) = delete;
  BluetoothLEAdvertisement& operator=(const BluetoothLEAdvertisement&) = delete;
  BluetoothLEAdvertisement& operator=(BluetoothLEAdvertisement&&) = delete;

private:
  ILogger& logger_;

  sdbus::InterfaceName le_advertisement_interface_name_;
  std::unique_ptr<sdbus::IObject> le_advertisement_;
};
}  // namespace screen_controller::dbus
