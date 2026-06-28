// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.hpp"

#include <models/app_settings.hpp>

#include "models/packet.hpp"
#include "socket/socket_variables.hpp"
#include "unpacker/unpacker.hpp"

namespace screen_controller::bluetooth {

BluetoothManager::BluetoothManager(ILogger& logger, const AppSettings& settings,
                                   IEventManager& events)
    : settings_(settings),
      logger_(logger),
      events_(events),
      l2_cap_receiver_(logger, settings),
      dbus_manager_(dbus::DbusManager(logger)),
      connection_state_(ConnectionState::kStarting) {
  // Register socket lifecycle callbacks.
  l2_cap_receiver_.OnConnected([this] {
    logger_.LogInfo("Phone connected — disabling discoverability");
    connection_state_ = ConnectionState::kConnecting;
    events_.Publish(ConnectionChangedEvent{.connected = true});
  });

  l2_cap_receiver_.OnAuthenticated([this] {
    logger_.LogInfo("Phone authenticated — locking adapter");
    dbus_manager_.DisableNewConnection();
    connection_state_ = ConnectionState::kConnected;
  });

  l2_cap_receiver_.OnDisconnected([this] {
    logger_.LogInfo("Phone disconnected — restoring discoverability");
    connection_state_ = ConnectionState::kStarting;
    dbus_manager_.MakeConnectable();
    events_.Publish(ConnectionChangedEvent{.connected = false});
  });

  l2_cap_receiver_.OnPacket([this](const Packet& raw) {
    if (raw.has_payload_) {
      BluetoothPacket packet{};
      packet.name = raw.name_;
      const Unpacker unpacker(logger_);
      unpacker.Decompress(raw.payload_, packet);
      events_.Publish(FileReceivedEvent{.filename = packet.name, .data = packet.data});
    } else {
      events_.Publish(CommandReceivedEvent{.command = raw.name_});
    }
  });

  // Start discoverable so the phone can find the device.
  dbus_manager_.MakeConnectable();

  logger_.LogInfo("BluetoothManager ready — device is discoverable");
}

std::expected<std::unique_ptr<BluetoothManager>, std::error_code> BluetoothManager::Create(
    ILogger& logger, const AppSettings& settings, IEventManager& events) {
  try {
    return std::unique_ptr<BluetoothManager>(new BluetoothManager(logger, settings, events));
  } catch (const std::exception& e) {
    logger.LogFmt(LogLevel::ERROR, "BluetoothManager::Create failed: {}", e.what());
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::Poll() {
  l2_cap_receiver_.PollSocket();
}

bool BluetoothManager::SendPacket(const uint8_t kType, const std::string_view kName,
                                  std::span<const std::byte> payload) {
  if (!l2_cap_receiver_.IsAuthenticated()) {
    logger_.LogWarn("SendPacket: no authenticated client");
    return false;
  }
  return l2_cap_receiver_.SendPacket(kType, kName, payload);
}

}  // namespace screen_controller::bluetooth
