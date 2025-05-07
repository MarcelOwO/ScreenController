//
// Created by marce on 5/2/2025.
//

#ifndef BLUETOOTH_AGENT_H
#define BLUETOOTH_AGENT_H
#include <memory>

#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IObject.h"
#include "sdbus-c++/Types.h"
namespace screen_controller::dbus {
class BluetoothAgent {
 public:
  BluetoothAgent(const std::shared_ptr<sdbus::IConnection>& connection,
                 sdbus::ObjectPath  device);
  ~BluetoothAgent() = default;
  BluetoothAgent(const BluetoothAgent&) = delete;
  BluetoothAgent& operator=(const BluetoothAgent&) = delete;
  BluetoothAgent(BluetoothAgent&&) = delete;
  BluetoothAgent& operator=(BluetoothAgent&&) = delete;

  void Setup(
      const std::function<void()>& release,
      const std::function<std::string(sdbus::ObjectPath)>& request_pin_code,
      const std::function<void(sdbus::ObjectPath, std::string pincode)>&
          display_pin_code,
      const std::function<uint32_t(sdbus::ObjectPath)>& request_passkey,
      const std::function<void(sdbus::ObjectPath, uint32_t, uint16_t)>&
          display_passkey,
      const std::function<void(sdbus::ObjectPath, uint32_t)>&
          request_confirmation,
      const std::function<void(sdbus::ObjectPath)>& request_authorization,
      const std::function<void(sdbus::ObjectPath, std::string)>&
          authorize_service,
      const std::function<void()>& cancel) const;

 private:

  sdbus::ObjectPath agent_path_;
  std::unique_ptr<sdbus::IObject> agent_object_;
};
}  // namespace screen_controller::dbus
#endif  // BLUETOOTH_AGENT_H
