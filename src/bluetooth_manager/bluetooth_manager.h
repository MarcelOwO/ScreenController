//
// Created by marce on 4/2/2025.
//

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H


#include <functional>

#include "../common/bluetooth_packet.h"
#include "dbus/dbus_manager.h"
#include "socket/l2cap_receiver.h"

namespace screen_controller::bluetooth {

class BluetoothManager {
 public:
  BluetoothManager();
  ~BluetoothManager();

  BluetoothManager(const BluetoothManager&) = delete;
  BluetoothManager& operator=(const BluetoothManager&) = delete;
  BluetoothManager(BluetoothManager&&) = delete;
  BluetoothManager& operator=(BluetoothManager&&) = delete;

  bool init();
  void run();

  void on_packet_received(
      std::function<void(const common::BluetoothPacket& packet)> callback);

 private:
  L2CapReceiver l2_cap_receiver_;
  std::unique_ptr<dbus::DbusManager> dbus_manager_;
  std::function<void(const common::BluetoothPacket& packet)> bluetooth_callback_;
};
}  // namespace screen_controller::bluetooth

#endif  // BLUETOOTH_MANAGER_H
