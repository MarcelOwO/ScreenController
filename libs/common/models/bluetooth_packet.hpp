//
// Created by marce on 5/31/2025.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../helper/define.hpp"

namespace screen_controller {

struct BluetoothPacket {
  u8 type;
  std::string name;
  std::size_t size;
  std::vector<std::byte> data;
};
}  // namespace screen_controller
