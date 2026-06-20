#pragma once

#include <logging/logger.hpp>
#include "sdbus-c++/sdbus-c++.h"

namespace screen_controller::dbus {

class ObjectManager : public sdbus::AdaptorInterfaces<sdbus::ObjectManager_adaptor> {
public:
  ObjectManager(ILogger& logger, sdbus::IConnection& connection, sdbus::ObjectPath path);

  ~ObjectManager();
  ObjectManager(const ObjectManager&) = delete;
  ObjectManager(ObjectManager&&) = delete;
  ObjectManager& operator=(const ObjectManager&) = delete;
  ObjectManager& operator=(ObjectManager&&) = delete;

private:
  ILogger& logger_;
  sdbus::IConnection& connection_;
  std::unique_ptr<sdbus::IObject> object_manager_object_;
};

}  // namespace screen_controller::dbus
