#pragma once

#include <string>
#include "../helper/define.hpp"

namespace screen_controller {

struct AppSettings {
  std::string app_name_;
  u32 window_width_;
  u32 window_height_;
  f32 rotation_amount_;
  u32 omtu_;
  u32 imtu_;
};

}  // namespace screen_controller
