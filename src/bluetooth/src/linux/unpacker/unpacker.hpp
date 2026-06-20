//
// Created by marce on 5/31/2025.
//

#pragma once

#include <logging/logger.hpp>
#include <models/bluetooth_packet.hpp>

#include <span>

namespace screen_controller::bluetooth {

class Unpacker final {
public:
  explicit Unpacker(ILogger& logger);
  ~Unpacker();

  void Decompress(std::span<const std::byte> span, BluetoothPacket& packet) const;

  Unpacker(const Unpacker&) = delete;
  Unpacker& operator=(const Unpacker&) = delete;
  Unpacker(Unpacker&&) = delete;
  Unpacker& operator=(Unpacker&&) = delete;

private:
  ILogger& logger_;
};

}  // namespace screen_controller::bluetooth
