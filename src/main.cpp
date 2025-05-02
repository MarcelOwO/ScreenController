#include <iostream>

#include "App.h"

int main() {
  (void)setenv("DISPLAY", ":0", 1);

  try {
    screen_controller::App app;
    if (!app.init()) {
      std::cerr << "Failed to initialize app" << std::endl;
      throw std::runtime_error("Failed to initialize app");
    }

    app.run();

  } catch (const std::exception& e) {
    std::cerr << "Unhandled exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
