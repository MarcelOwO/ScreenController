#include <cstdlib>
#include <iostream>
#include <models/app_settings.hpp>
#include "app/app.hpp"

int main() {
  // Respect a compositor/session selected by the service or caller.
  if (std::getenv("DISPLAY") == nullptr) {
    (void) setenv("DISPLAY", ":0", 0);
  }

  try {
    screen_controller::App app;

    app.Run();
  } catch (std::exception& e) {
    std::cerr << "Application failed:" << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
