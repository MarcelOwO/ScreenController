#include <cstdlib>
#include <iostream>
#include <models/app_settings.hpp>
#include "app/app.hpp"

int main(int argc, char* argv[]) {
  // hack for launchin this from ssh
  (void) setenv("DISPLAY", ":0", 1);

  try {
    screen_controller::App app;

    app.AdjustSettings([](screen_controller::AppSettings& settings) {
      settings.app_name_ = "ScreenController";
      settings.imtu_ = 672;
      settings.omtu_ = 672;
      settings.rotation_amount_ = 90;
      settings.window_height_ = 1080;
      settings.window_width_ = 1920;
    });

    app.Run();
  } catch (std::exception& e) {
    std::cerr << "Application failed:" << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
