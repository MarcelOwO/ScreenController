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
    logger_.LogInfo("Phone connected — awaiting authentication");
    connection_state_ = ConnectionState::kAuthenticating;
    events_.Publish(ConnectionChangedEvent{true});
  });

  l2_cap_receiver_.OnAuthenticated([this] {
    logger_.LogInfo("Phone authenticated — locking adapter");
    dbus_manager_.DisableNewConnection();
    connection_state_ = ConnectionState::kConnected;
  });

  l2_cap_receiver_.OnDisconnected([this] {
    logger_.LogInfo("Phone disconnected — restoring controller advertisement");
    connection_state_ = ConnectionState::kStarting;
    dbus_manager_.MakeConnectable(!l2_cap_receiver_.IsCommissioned());
    events_.Publish(ConnectionChangedEvent{false});
  });

  l2_cap_receiver_.OnPacket([this](const Packet& raw) {
    if (raw.type_ == socket::kTypeCommand && !raw.has_payload_) {
      events_.Publish(CommandReceivedEvent{raw.name_});
      return;
    }

    if (raw.type_ != socket::kTypeFileTransfer || !raw.has_payload_) {
      logger_.LogFmt(LogLevel::WARN, "Unsupported packet type: {}", raw.type_);
      l2_cap_receiver_.SendError(0x04, "unsupported packet type");
      return;
    }

    constexpr std::size_t kMaxDecompressedFileSize = 128U * 1024U * 1024U;
    const Unpacker unpacker;
    auto data = unpacker.Decompress(raw.payload_, kMaxDecompressedFileSize);
    if (!data) {
      logger_.LogFmt(LogLevel::ERROR, "Rejected file transfer: {}", data.error());
      l2_cap_receiver_.SendError(0x05, data.error());
      return;
    }
    events_.Publish(FileReceivedEvent{raw.name_, std::move(*data)});
  });

  l2_cap_receiver_.OnError([this](const int code, const std::string_view message) {
    logger_.LogFmt(LogLevel::ERROR, "Bluetooth socket error {}: {}", code, message);
  });

  // An uncommissioned device allows pairing. After the first authenticated controller claims the
  // device, it advertises for that bonded controller but never becomes pairable again.
  dbus_manager_.MakeConnectable(!l2_cap_receiver_.IsCommissioned());

  logger_.LogInfo(l2_cap_receiver_.IsCommissioned()
                      ? "BluetoothManager ready — device is locked to its controller"
                      : "BluetoothManager ready — device is available for commissioning");
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
