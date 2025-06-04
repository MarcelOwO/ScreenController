//
// Created by marce on 5/2/2025.
//

#ifndef BLUETOOTH_AGENT_H
#define BLUETOOTH_AGENT_H
#include <memory>

#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IObject.h"
#include "sdbus-c++/Types.h"
namespace screen_controller::bluetooth::dbus {
class BluetoothAgent {
 public:
  BluetoothAgent(const std::shared_ptr<sdbus::IConnection>& connection,
                 sdbus::ObjectPath device);
  ~BluetoothAgent() = default;
  BluetoothAgent(const BluetoothAgent&) = delete;
  BluetoothAgent& operator=(const BluetoothAgent&) = delete;
  BluetoothAgent(BluetoothAgent&&) = delete;
  BluetoothAgent& operator=(BluetoothAgent&&) = delete;

  void init() const;

 private:
  sdbus::ObjectPath agent_path_;
  std::unique_ptr<sdbus::IObject> agent_object_;
  std::shared_ptr<sdbus::IConnection> connection_;
};
}  // namespace screen_controller::dbus
#endif  // BLUETOOTH_AGENT_H
