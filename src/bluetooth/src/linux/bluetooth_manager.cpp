// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.hpp"

#include <models/app_settings.hpp>

#include "models/packet.hpp"
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

  logger_.LogInfo("BluetoothManager ready");
}

std::expected<std::unique_ptr<BluetoothManager>, std::error_code> BluetoothManager::Create(
    ILogger& logger, const AppSettings& settings, IEventManager& events) {
  try {
    auto manager =
        std::unique_ptr<BluetoothManager>(new BluetoothManager(logger, settings, events));
    return manager;
  } catch (const std::exception& e) {
    logger.LogFmt(LogLevel::ERROR, "BluetoothManager::Create failed: {}", e.what());
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }
}

BluetoothManager::~BluetoothManager() = default;

void BluetoothManager::Poll() {
  l2_cap_receiver_.PollSocket();
}

}  // namespace screen_controller::bluetooth
