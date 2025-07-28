#include <ng-log/logging.h>

#include <iostream>

#include "App.h"

int main(int argc, char* argv[]) {
  (void)setenv("DISPLAY", ":0", 1);

  nglog::InitializeLogging(argv[0]);
  FLAGS_logtostderr = true;

  LOG(INFO) << "Starting Screen Controller App";

  screen_controller::App app;

  CHECK(app.init()) << "Failed to initialize app";

  app.run();

  return EXIT_SUCCESS;
}