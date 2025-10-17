//
// Created by marce on 4/2/2025.
//

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <expected>
#include <functional>
#include <memory>

#include "bluetooth_packet.h"
#include "dbus/dbus_manager.h"
#include "logging/logger.h"
#include "models/error_enum.h"
#include "socket/l2cap_receiver.h"

namespace screen_controller::bluetooth {

class BluetoothManager {
 public:
  explicit BluetoothManager(std::shared_ptr<Logger> logger);
  ~BluetoothManager();

  BluetoothManager(const BluetoothManager&) = delete;
  BluetoothManager& operator=(const BluetoothManager&) = delete;
  BluetoothManager(BluetoothManager&&) = delete;
  BluetoothManager& operator=(BluetoothManager&&) = delete;

  [[nodiscard]] std::expected<void, common::ErrorEnum> init();
  void run();

  void on_packet_received(
      std::function<void(const common::BluetoothPacket& packet)> callback);

 private:
  std::shared_ptr<Logger> logger_;
  L2CapReceiver l2_cap_receiver_;
  std::unique_ptr<dbus::DbusManager> dbus_manager_;
  std::function<void(const common::BluetoothPacket& packet)>
      bluetooth_callback_;
};
}  // namespace screen_controller::bluetooth

#endif  // BLUETOOTH_MANAGER_H
