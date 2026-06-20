#include "object_manager.hpp"

#include "sdbus-c++/StandardInterfaces.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::dbus {

ObjectManager::ObjectManager(ILogger& logger, sdbus::IConnection& connection,
                             sdbus::ObjectPath path)
    : AdaptorInterfaces(connection, std::move(path)), logger_(logger), connection_(connection) {
  registerAdaptor();

  logger_.LogInfo("Creating ObjectManager");
}

ObjectManager::~ObjectManager() {
  unregisterAdaptor();
};

}  // namespace screen_controller::dbus
