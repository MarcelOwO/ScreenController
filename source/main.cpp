
#include <app/app.h>

#include <exception>

int main(int argc, char* argv[]) {
  try {
    screen_controller::App app;
    app.settings = screen_controller::AppSettings{
        .app_name = "ScreenControllerApp",
    };

    app.run();

  } catch (const std::exception& e) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
