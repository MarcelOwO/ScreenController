//
// Created by marce on 4/2/2025.
//

#include "App.h"

#include <iostream>
#include <map>


App::App() : window_manager_(),
             renderer_(),
             bluetooth_manager_(),
             storage_manager_(), file_processor_() {
}

App::~App() {
    cleanup();
}

void App::init() {
    try {
        const auto map = std::map<std::string, Command>{
            {"SET_SCREEN", Command::SET_SCREEN},
            {"DELETE_FILE", Command::DELETE_FILE}
        };

        storage_manager_.init();
        bluetooth_manager_.init();

        bluetooth_manager_.on_command_received(
            [map](const std::string& command, const std::vector<std::byte>& data) {
                if (map.contains(command)) {
                    switch (map.at(command)) {
                    case Command::SET_SCREEN:
                        //auto file = storage_manager_.load_file(command,data);
                        //renderer_.set_texture(file);
                        break;
                    case Command::DELETE_FILE:
                        //storage_manager_.delete();
                        break;
                    case Command::NONE: break;
                    }
                }
            });


        bluetooth_manager_.on_file_received(
            [this](const std::string& name, const std::vector<std::byte>& data) {
                storage_manager_.save_file(name, data);
            });

        window_manager_.init();
        renderer_.init(std::bit_cast<GLADloadproc>(window_manager_.address_pointer()),
                       window_manager_.get_width(),
                       window_manager_.get_height());
    }

    catch
    (

        const std::exception& e
    ) {
        std::cout << e.what() << std::endl;
    }
}

void App::run() {
    while (!window_manager_.should_close()) {
        bluetooth_manager_.run();
        storage_manager_.run();
        file_processor_.run();

        if (auto frame = file_processor_.next_frame(); frame.has_value()) {
            renderer_.set_texture(frame.value());
        }

        window_manager_.update([this] { renderer_.render(); });
        window_manager_.poll_events();
    }
}

void App::cleanup() {
    window_manager_.cleanup();
    bluetooth_manager_.cleanup();
    storage_manager_.cleanup();
    renderer_.cleanup();
    file_processor_.cleanup();
}
