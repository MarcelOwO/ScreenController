
#include "gatt_service.hpp"

#include "sdbus-c++/Types.h"

namespace screen_controller::dbus {

GattService::GattService(ILogger& logger, sdbus::IConnection& connection)
    : logger_(logger),
      connection_(connection),
      gatt_service_object_(
          sdbus::createObject(connection_, sdbus::ObjectPath("/owo/gatt_service"))) {
  logger_.LogInfo("Creating GattService");

  gatt_service_object_
      ->addVTable(sdbus::registerProperty("UUID").withSetter(
                      []() { return "0000180d-0000-1000-8000-00805f9b34fb"; }),
                  sdbus::registerProperty("Primary").withSetter([]() { return true; }))
      .forInterface(sdbus::InterfaceName("org.bluez.GattService1"));
}

}  // namespace screen_controller::dbus
