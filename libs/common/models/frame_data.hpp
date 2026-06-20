//
// Created by marce on 5/6/2025.
//
#pragma once
#include <cstdint>
#include <vector>
#include "helper/define.hpp"

namespace screen_controller {

struct FrameData {
  std::vector<u8> data;
  i32 width;
  i32 height;
  i32 channels;
};

}  // namespace screen_controller
