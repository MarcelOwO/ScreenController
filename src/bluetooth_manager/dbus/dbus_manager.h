//
// Created by marce on 05/08/2025.
//

#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H

#include <logging/logger.h>

#include <memory>
#include <string_view>

#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller {

class DbusManager {
 public:
  explicit DbusManager(const std::shared_ptr<Logger>& logger,
                       std::string_view alias);
  ~DbusManager() = default;

  DbusManager(const DbusManager&) = delete;
  DbusManager& operator=(const DbusManager&) = delete;
  DbusManager(DbusManager&&) = delete;
  DbusManager& operator=(DbusManager&&) = delete;

  void poll_adapters();
  void setup_adapter() const;

 private:
  std::string alias_;
  std::shared_ptr<Logger> logger_;

  std::shared_ptr<sdbus::IConnection> connection_;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  std::shared_ptr<sdbus::IProxy> bluez_proxy_;
  sdbus::ObjectPath advertisement_path_;
  sdbus::ObjectPath agent_path_;
  std::chrono::time_point<std::chrono::steady_clock> last_time_point_;
};
}  // namespace screen_controller

#endif  // DBUSMANAGER_H
