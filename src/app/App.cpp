//
// Created by marce on 4/2/2025.
//

#include "App.h"

#include <iostream>


App::App() {
    try {
        window_ = std::make_unique<WindowManager>();
        renderer_ = std::make_unique<GraphicsRenderer>();
        bluetooth_ = std::make_unique<BluetoothManager>();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

App::~App() {
    cleanup();
}

void App::init() const {
    try {
        bluetooth_->init();
        window_->init();
        renderer_->init(std::bit_cast<GLADloadproc>(window_->address_pointer()),
                        window_->get_width(),
                        window_->get_height());
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

void App::run() const {
    while (!window_->should_close()) {
        window_->update([this] { renderer_->render(); });
        window_->poll_events();
    }
}

void App::cleanup() const {
    window_->cleanup();
}
