//
// Created by marce on 4/2/2025.
//

#pragma once

#include <bt/manager.hpp>

#include <events/events.hpp>
#include <logging/logger.hpp>
#include <models/app_settings.hpp>
#include <models/bluetooth_packet.hpp>

#include <mutex>
#include <vector>

#include "dbus/dbus_manager.hpp"
#include "models/connection_state.hpp"
#include "models/packet.hpp"
#include "socket/l2cap_receiver.hpp"

namespace screen_controller::bluetooth {

class BluetoothManager : public IBluetoothManager {
public:
  ~BluetoothManager() override;

  static std::expected<std::unique_ptr<BluetoothManager>, std::error_code> Create(
      ILogger& logger, const AppSettings& settings, IEventManager& events);

  BluetoothManager(const BluetoothManager&) = delete;
  BluetoothManager& operator=(const BluetoothManager&) = delete;
  BluetoothManager(BluetoothManager&&) = delete;
  BluetoothManager& operator=(BluetoothManager&&) = delete;

  void Poll() override;
  bool SendPacket(uint8_t type, std::string_view name,
                  std::span<const std::byte> payload = {}) override;

private:
  enum class ActiveTransport { kNone, kL2Cap, kGatt };
  enum class GattEventType { kConnected, kAuthenticated, kDisconnected, kPacket };

  struct GattEvent {
    GattEventType type;
    uint8_t packet_type{0};
    std::string name;
    std::vector<std::byte> payload;
    bool has_payload{false};
  };

  explicit BluetoothManager(ILogger& logger, const AppSettings& settings, IEventManager& events);

  void HandleConnected(ActiveTransport transport);
  void HandleAuthenticated(ActiveTransport transport);
  void HandleDisconnected(ActiveTransport transport);
  void HandlePacket(const Packet& packet);
  void QueueGattEvent(GattEvent event);
  void DrainGattEvents();

  const AppSettings& settings_;
  ILogger& logger_;
  IEventManager& events_;

  socket::L2CapReceiver l2_cap_receiver_;
  dbus::DbusManager dbus_manager_;

  ConnectionState connection_state_;
  ActiveTransport active_transport_{ActiveTransport::kNone};
  std::mutex gatt_events_mutex_;
  std::vector<GattEvent> gatt_events_;
};

}  // namespace screen_controller::bluetooth
