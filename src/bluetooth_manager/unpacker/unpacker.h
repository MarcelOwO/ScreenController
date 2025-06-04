//
// Created by marce on 5/31/2025.
//

#ifndef UNPACKER_H
#define UNPACKER_H
#include <span>

#include "../common/bluetooth_packet.h"

namespace screen_controller::bluetooth {
class Unpacker final {
 public:
  Unpacker();
  virtual ~Unpacker();

  void decompress(const std::span<std::byte> span,
                  common::BluetoothPacket &packet);

  Unpacker(const Unpacker &) = delete;
  Unpacker &operator=(const Unpacker &) = delete;
  Unpacker(Unpacker &&) = delete;
  Unpacker &operator=(Unpacker &&) = delete;

  void init();

 private:
};
}  // namespace screen_controller::bluetooth

#endif  // UNPACKER_H
