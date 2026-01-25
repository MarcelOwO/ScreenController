
#include <app/app.h>

int main(int argc, char* argv[]) {
  screen_controller::App app;

  app.settings = screen_controller::AppSettings{
      .app_name = "ScreenControllerApp",
  };

  app.run();

  return -1;
}
