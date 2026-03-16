#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <string>

namespace screen_controller {

struct AppSettings {
  std::string app_name_;
  int window_width_;
  int window_height_;
  float rotation_amount_;
};

}  // namespace screen_controller
#endif  // APP_SETTINGS_H
