//
// Created by marce on 4/2/2025.
//

#ifndef BLUETOOTH_MANAGER_IMPL_H
#define BLUETOOTH_MANAGER_IMPL_H

#include <app_settings.h>
#include <bluetooth_packet.h>
#include <bt/manager.h>
#include <logging/logger.h>

#include <functional>

#include "dbus/dbus_manager.h"
#include "models/connection_state.h"
#include "socket/l2cap_receiver.h"

namespace screen_controller {

class BluetoothManager : public IBluetoothManager {
 public:
  explicit BluetoothManager(
      ILogger& logger, const AppSettings& settings,
      const std::function<void(const common::BluetoothPacket& packet)>&
          callback);
  ~BluetoothManager();

  BluetoothManager(const BluetoothManager&) = delete;
  BluetoothManager& operator=(const BluetoothManager&) = delete;
  BluetoothManager(BluetoothManager&&) = delete;
  BluetoothManager& operator=(BluetoothManager&&) = delete;

  void poll();

 private:
  const AppSettings& settings_;
  ILogger& logger_;

  L2CapReceiver l2_cap_receiver_;
  DbusManager dbus_manager_;

  ConnectionState connection_state_;

  std::function<void(const common::BluetoothPacket& packet)>
      bluetooth_callback_;
};
}  // namespace screen_controller

#endif  // BLUETOOTH_MANAGER_H
