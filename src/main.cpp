#include <iostream>
#include "app/app.h"

int main(int argc, char* argv[]) {
  // hack for launchin this from ssh
  (void) setenv("DISPLAY", ":0", 1);

  try {
    screen_controller::App app;
    app.Run();
  } catch (std::exception& e) {
    std::cerr << "Application failed:" << e.what() << '\n';
    return -1;
  }
  return 0;
}
