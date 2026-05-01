//
// Created by marce on 5/31/2025.
//

#ifndef UNPACKER_H
#define UNPACKER_H
#include <bluetooth_packet.h>
#include <logging/logger.h>
#include <models/error_enum.h>

#include <span>

namespace screen_controller {
class Unpacker final {
public:
  explicit Unpacker(ILogger& logger);
  virtual ~Unpacker();

  void Decompress(std::span<std::byte> span, common::BluetoothPacket& packet) const;

  Unpacker(const Unpacker&) = delete;
  Unpacker& operator=(const Unpacker&) = delete;
  Unpacker(Unpacker&&) = delete;
  Unpacker& operator=(Unpacker&&) = delete;

private:
  ILogger& logger_;
};
}  // namespace screen_controller

#endif  // UNPACKER_H
