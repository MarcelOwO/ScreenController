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
  auto& gatt = dbus_manager_.Gatt();

  l2_cap_receiver_.OnConnected([this] { HandleConnected(ActiveTransport::kL2Cap); });
  l2_cap_receiver_.OnAuthenticated([this] { HandleAuthenticated(ActiveTransport::kL2Cap); });
  l2_cap_receiver_.OnDisconnected([this] { HandleDisconnected(ActiveTransport::kL2Cap); });
  l2_cap_receiver_.OnPacket([this](const Packet& packet) {
    if (active_transport_ == ActiveTransport::kL2Cap) {
      HandlePacket(packet);
    }
  });

  gatt.OnConnected([this] { QueueGattEvent(GattEvent{.type = GattEventType::kConnected}); });
  gatt.OnAuthenticated(
      [this] { QueueGattEvent(GattEvent{.type = GattEventType::kAuthenticated}); });
  gatt.OnDisconnected([this] { QueueGattEvent(GattEvent{.type = GattEventType::kDisconnected}); });
  gatt.OnPacket([this](const Packet& packet) {
    QueueGattEvent(GattEvent{.type = GattEventType::kPacket,
                             .packet_type = packet.type_,
                             .name = packet.name_,
                             .payload = std::vector(packet.payload_.begin(), packet.payload_.end()),
                             .has_payload = packet.has_payload_});
  });

  l2_cap_receiver_.OnError([this](const int code, const std::string_view message) {
    logger_.LogFmt(LogLevel::ERROR, "Bluetooth socket error {}: {}", code, message);
  });

  // An uncommissioned device allows pairing. After the first authenticated controller claims the
  // device, it advertises for that bonded controller but never becomes pairable again.
  const bool commissioned = l2_cap_receiver_.IsCommissioned() || gatt.IsCommissioned();
  dbus_manager_.MakeConnectable(!commissioned);

  logger_.LogInfo(commissioned ? "BluetoothManager ready — device is locked to its controller"
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

void BluetoothManager::HandleConnected(const ActiveTransport transport) {
  if (active_transport_ != ActiveTransport::kNone && active_transport_ != transport) {
    logger_.LogWarn("Ignoring a second Bluetooth transport while a controller is active");
    return;
  }
  active_transport_ = transport;
  logger_.LogInfo("Phone connected — awaiting authentication");
  connection_state_ = ConnectionState::kAuthenticating;
  events_.Publish(ConnectionChangedEvent{true});
}

void BluetoothManager::HandleAuthenticated(const ActiveTransport transport) {
  if (active_transport_ != transport) {
    return;
  }
  logger_.LogInfo("Phone authenticated — locking adapter");
  dbus_manager_.DisableNewConnection();
  connection_state_ = ConnectionState::kConnected;
}

void BluetoothManager::HandleDisconnected(const ActiveTransport transport) {
  if (active_transport_ != transport) {
    return;
  }
  logger_.LogInfo("Phone disconnected — restoring controller advertisement");
  active_transport_ = ActiveTransport::kNone;
  connection_state_ = ConnectionState::kStarting;
  const bool commissioned =
      l2_cap_receiver_.IsCommissioned() || dbus_manager_.Gatt().IsCommissioned();
  dbus_manager_.MakeConnectable(!commissioned);
  events_.Publish(ConnectionChangedEvent{false});
}

void BluetoothManager::HandlePacket(const Packet& packet) {
  if (packet.type_ == socket::kTypeCommand && !packet.has_payload_) {
    events_.Publish(CommandReceivedEvent{packet.name_});
    return;
  }

  if (packet.type_ != socket::kTypeFileTransfer || !packet.has_payload_) {
    logger_.LogFmt(LogLevel::WARN, "Unsupported packet type: {}", packet.type_);
    constexpr std::string_view kError{"ERR:4:unsupported packet type"};
    (void) SendPacket(socket::kTypeDeviceError, "error",
                      std::as_bytes(std::span{kError.data(), kError.size()}));
    return;
  }

  constexpr std::size_t kMaxDecompressedFileSize = 128U * 1024U * 1024U;
  const Unpacker unpacker;
  auto data = unpacker.Decompress(packet.payload_, kMaxDecompressedFileSize);
  if (!data) {
    logger_.LogFmt(LogLevel::ERROR, "Rejected file transfer: {}", data.error());
    const auto error = std::format("ERR:5:{}", data.error());
    (void) SendPacket(socket::kTypeDeviceError, "error",
                      std::as_bytes(std::span{error.data(), error.size()}));
    return;
  }
  events_.Publish(FileReceivedEvent{packet.name_, std::move(*data)});
}

void BluetoothManager::QueueGattEvent(GattEvent event) {
  const std::lock_guard lock(gatt_events_mutex_);
  gatt_events_.push_back(std::move(event));
}

void BluetoothManager::DrainGattEvents() {
  std::vector<GattEvent> events;
  {
    const std::lock_guard lock(gatt_events_mutex_);
    events.swap(gatt_events_);
  }
  for (auto& event : events) {
    switch (event.type) {
      case GattEventType::kConnected:
        HandleConnected(ActiveTransport::kGatt);
        break;
      case GattEventType::kAuthenticated:
        HandleAuthenticated(ActiveTransport::kGatt);
        break;
      case GattEventType::kDisconnected:
        HandleDisconnected(ActiveTransport::kGatt);
        break;
      case GattEventType::kPacket:
        if (active_transport_ == ActiveTransport::kGatt) {
          HandlePacket(Packet{.type_ = event.packet_type,
                              .name_ = std::move(event.name),
                              .payload_ = event.payload,
                              .has_payload_ = event.has_payload});
        }
        break;
    }
  }
}

void BluetoothManager::Poll() {
  dbus_manager_.Poll();
  l2_cap_receiver_.PollSocket();
  DrainGattEvents();
}

bool BluetoothManager::SendPacket(const uint8_t kType, const std::string_view kName,
                                  std::span<const std::byte> payload) {
  switch (active_transport_) {
    case ActiveTransport::kL2Cap:
      return l2_cap_receiver_.SendPacket(kType, kName, payload);
    case ActiveTransport::kGatt:
      return dbus_manager_.Gatt().SendPacket(kType, kName, payload);
    case ActiveTransport::kNone:
      logger_.LogWarn("SendPacket: no authenticated client");
      return false;
  }
  return false;
}

}  // namespace screen_controller::bluetooth
