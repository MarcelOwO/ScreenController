//
// Created by marce on 05/08/2025.
//

#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H

#include <logging/logger.h>

#include <memory>

#include "dbus_implementations/bluetooth_adapter.h"
#include "dbus_implementations/bluetooth_agent.h"
#include "dbus_implementations/bluetooth_agent_manager.h"
#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller {

class DbusManager {
public:
  explicit DbusManager(ILogger& logger);

  ~DbusManager();

  DbusManager(const DbusManager&) = delete;
  DbusManager& operator=(const DbusManager&) = delete;
  DbusManager(DbusManager&&) = delete;
  DbusManager& operator=(DbusManager&&) = delete;

  void MakeConnectable();
  void DisableNewConnection();

private:
  sdbus::ServiceName service_name_;
  std::string alias_;
  ILogger& logger_;

  std::shared_ptr<sdbus::IConnection> connection_;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  std::shared_ptr<sdbus::IProxy> bluez_proxy_;

  sdbus::ObjectPath advertisement_path_;
  sdbus::ObjectPath agent_path_;

  BluetoothAgentManager agent_manager_;
  BluetoothAgent agent_;
  BluetoothAdapter adapter_;

  void SetupAgents();
  void SetupName();
  void SetupAdapter();
};
}  // namespace screen_controller

#endif  // DBUSMANAGER_H
