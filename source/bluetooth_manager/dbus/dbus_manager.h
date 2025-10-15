//
// Created by marce on 05/08/2025.
//

#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H

#include <memory>

#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::bluetooth::dbus {

class DbusManager {
 public:
  DbusManager();
  ~DbusManager() = default;
  DbusManager(const DbusManager&) = delete;
  DbusManager& operator=(const DbusManager&) = delete;
  DbusManager(DbusManager&&) = delete;
  DbusManager& operator=(DbusManager&&) = delete;

  bool init();

  void poll_adapters();

 private:
  std::shared_ptr<sdbus::IConnection> connection_= nullptr;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_= nullptr;
  std::shared_ptr<sdbus::IProxy> bluez_proxy_ = nullptr;
  sdbus::ObjectPath advertisement_path_{sdbus::ObjectPath("/org/bluez/hci0/owo")};
  sdbus::ObjectPath agent_path_{sdbus::ObjectPath("/org/bluez/hci0/owo/agent1")};
  std::chrono::time_point<std::chrono::steady_clock> last_time_point_ = std::chrono::steady_clock::now();
};
}  // namespace screen_controller::bluetooth::dbus

#endif  // DBUSMANAGER_H
