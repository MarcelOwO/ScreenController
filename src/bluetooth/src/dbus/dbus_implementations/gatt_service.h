#ifndef GATT_SERVICE_H
#define GATT_SERVICE_H

#include <logging/logger.h>

#include "sdbus-c++/AdaptorInterfaces.h"
#include "sdbus-c++/IConnection.h"

namespace screen_controller {

class GattService {
 public:
  explicit GattService(ILogger& logger, sdbus::IConnection& connection);
  ~GattService() = default;

  GattService(const GattService&) = delete;
  GattService& operator=(const GattService&) = delete;
  GattService(GattService&&) = delete;
  GattService& operator=(GattService&&) = delete;

 private:
  ILogger& logger_;
  sdbus::IConnection& connection_;

  std::unique_ptr<sdbus::IObject> gatt_service_object_;
};

};  // namespace screen_controller

#endif
