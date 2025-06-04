//
// Created by marce on 5/3/2025.
//

#include "bluetooth_le_advertisement.h"

#include <iostream>
namespace screen_controller::bluetooth::dbus {

BluetoothLEAdvertisement::BluetoothLEAdvertisement(
    const std::shared_ptr<sdbus::IConnection> &connection,
    const sdbus::ObjectPath &path)
    : le_advertisement_interface_name_(
          sdbus::InterfaceName("org.bluez.LEAdvertisement1")),
      le_advertisement_(sdbus::createObject(*connection, path)) {}

void BluetoothLEAdvertisement::init() const {
  le_advertisement_
      ->addVTable(sdbus::registerMethod("Release").withNoReply().implementedAs(
                      [=] { std::cout << "Released" << std::endl; }),
                  sdbus::registerProperty("Type").withGetter(
                      [=] { return std::string("peripheral"); }),
                  sdbus::registerProperty("ServiceUUIDs").withGetter([=] {
                    return std::vector<std::string>{
                        std::string("69696969-6969-6969-6969-696969696969")};
                  }))
      .forInterface(sdbus::InterfaceName(le_advertisement_interface_name_));
}

}  // namespace screen_controller::dbus
