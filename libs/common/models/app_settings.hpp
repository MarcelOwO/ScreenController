#pragma once

#include <string>
#include "../helper/define.hpp"

namespace screen_controller {

struct AppSettings {
  std::string app_name_{"ScreenController"};
  u32 window_width_{1920};
  u32 window_height_{1080};
  f32 rotation_amount_{90.0F};
  u32 omtu_{672};
  u32 imtu_{672};
};

}  // namespace screen_controller
