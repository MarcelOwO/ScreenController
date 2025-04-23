#include <iostream>

#include "App.h"

int main() {
    setenv("DISPLAY", ":0", 1); //hack

    try {
        App app;

        app.init();
        app.run();
        app.cleanup();
    }
    catch ([[maybe_unused]] const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
