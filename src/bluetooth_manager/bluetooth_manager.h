//
// Created by marce on 4/2/2025.
//

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <sdbus-c++/sdbus-c++.h>

#include <functional>
#include <string>
#include <vector>

#include "../common/bluetooth_packet.h"
#include "../common/command.h"
#include "socket/l_2_cap_receiver.h"

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

  void on_blutooth_packet_received(
      std::function<void(common::BluetoothPacket)> callback);

 private:
  L2CapReceiver l2_cap_receiver_;

  std::shared_ptr<sdbus::IConnection> connection_;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  std::shared_ptr<sdbus::IProxy> bluez_proxy;
  std::chrono::time_point<std::chrono::steady_clock> last_time_point_;

  sdbus::ObjectPath advertisement_path_;

  sdbus::ObjectPath agent_path_;

  std::function<void(screen_controller::common::BluetoothPacket)> bluetooth_callback_;
};
}  // namespace screen_controller::bluetooth

#endif  // BLUETOOTH_MANAGER_H
