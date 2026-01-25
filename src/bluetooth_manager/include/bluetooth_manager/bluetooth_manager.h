//
// Created by marce on 4/2/2025.
//

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <bluetooth_packet.h>

#include <functional>
#include <memory>

#include "../../models/connection_state.h"

#include "../../dbus/dbus_manager.h"
#include "../../socket/l2cap_receiver.h"
#include "app_settings.h"
#include "logging/logger.h"

namespace screen_controller {

class BluetoothManager {
 public:
  explicit BluetoothManager(const std::shared_ptr<Logger>& logger,const std::shared_ptr<AppSettings>& settings);
  ~BluetoothManager();

  BluetoothManager(const BluetoothManager&) = delete;
  BluetoothManager& operator=(const BluetoothManager&) = delete;
  BluetoothManager(BluetoothManager&&) = delete;
  BluetoothManager& operator=(BluetoothManager&&) = delete;

  void run();

  void on_packet_received(
      std::function<void(const common::BluetoothPacket& packet)> callback);

 private:
  std::shared_ptr<AppSettings> settings_;
  std::shared_ptr<Logger> logger_;

  L2CapReceiver l2_cap_receiver_;
  DbusManager dbus_manager_;

  ConnectionState connection_state_;

  std::function<void(const common::BluetoothPacket& packet)>
      bluetooth_callback_;


};
}  // namespace screen_controller

#endif  // BLUETOOTH_MANAGER_H
