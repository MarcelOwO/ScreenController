//
// Created by marce on 5/2/2025.
//

#pragma once

#include <logging/logger.hpp>

#include <memory>

#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IObject.h"
#include "sdbus-c++/Types.h"
namespace screen_controller::dbus {
class BluetoothAgent {
public:
  explicit BluetoothAgent(ILogger& logger, const std::shared_ptr<sdbus::IConnection>& connection,
                          const sdbus::ObjectPath& device);
  ~BluetoothAgent() = default;

  BluetoothAgent(const BluetoothAgent&) = delete;
  BluetoothAgent& operator=(const BluetoothAgent&) = delete;
  BluetoothAgent(BluetoothAgent&&) = delete;
  BluetoothAgent& operator=(BluetoothAgent&&) = delete;

  // void init() const;

private:
  ILogger& logger_;
  sdbus::ObjectPath agent_path_;
  std::unique_ptr<sdbus::IObject> agent_object_;
  std::shared_ptr<sdbus::IConnection> connection_;
};
}  // namespace screen_controller::dbus
