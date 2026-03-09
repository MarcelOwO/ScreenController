
#include "app/app.h"

int main(int argc, char* argv[]) {
  // hack for launchin this from ssh
  (void)setenv("DISPLAY", ":0", 1);

  screen_controller::App app;

  app.settings = screen_controller::AppSettings{
      .app_name = "ScreenControllerApp",
  };

  app.run();

  return -1;
}
