//
// Created by marce on 4/2/2025.
//

#include "App.h"

#include <iostream>

#include "command.h"

namespace screen_controller {

App::App() : running_(false) {}

App::~App() {
  running_ = false;
  queue_condition_.notify_all();

  if (command_thread_.joinable()) {
    command_thread_.join();
  }
}

bool App::init() {
  if (!storage_manager_.Init()) {
    std::cerr << "Failed to initialize storage manager." << std::endl;
    return false;
  }

  if (!bluetooth_manager_.init()) {
    std::cerr << "Failed to initialize bluetooth manager." << std::endl;
    return false;
  }

  if (!file_processor_.init()) {
    std::cerr << "Failed to initialize file processor." << std::endl;
    return false;
  }

  bluetooth_manager_.on_command_received(
      [this](const std::string& command, const std::vector<std::byte>& data) {
        {
          std::lock_guard<std::mutex> lock(queue_mutex_);
          (void)command_queue_.emplace(command, data);
        }
        queue_condition_.notify_one();
      });

  bluetooth_manager_.on_file_received(
      [this](const std::string& name, const std::vector<std::byte>& data) {
        if (!storage_manager_.SaveFile(name, data)) {
          std::cerr << "Failed to save file." << std::endl;
        }
      });

  if (!window_manager_.init()) {
    std::cerr << "Failed to initialize window manager." << std::endl;
    return false;
  }

  renderer_.init(std::bit_cast<GLADloadproc>(WindowManager::address_pointer()),
                 WindowManager::get_width(), WindowManager::get_height());

  if (!load_startup_image()) {
    std::cerr << "Failed to load startup image." << std::endl;
    return false;
  }

  running_ = true;

  return true;
}

bool App::load_startup_image() {
  auto path = storage_manager_.GetPath("sticker.webp");

  if (!file_processor_.process_file(path.c_str())) {
    std::cerr << "Failed to load sticker.webp" << std::endl;
    return false;
  }

  process_frame();
  return true;
}
void App::process_frame() {
  if (const auto frame = file_processor_.get_processed_data(); frame.has_value()) {
    renderer_.set_texture(frame.value());
  }
}

void App::render_loop() {
  while (!window_manager_.should_close()) {
    process_frame();
    window_manager_.update([this] { renderer_.render(); });
    WindowManager::poll_events();
  }
}
void App::handle_commands() {
  while (true) {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    queue_condition_.wait(lock, [this] { return !command_queue_.empty(); });
    auto [command, command_data] = command_queue_.front();
    command_queue_.pop();
    lock.unlock();
  }
}

void App::run() {
  std::thread command_thread(&App::handle_commands, this);
  command_thread_ = std::move(command_thread);
  render_loop();
  command_thread.join();
}

void App::command_callback(const std::string& command,
                           const std::vector<std::byte>& data) {
  {
    const auto map = std::unordered_map<std::string_view, common::Command>{
        {"SET_SCREEN", common::Command::kSetScreen},
        {"DELETE_FILE", common::Command::kDeleteFile}};

    const auto& command_type = command;
    const auto& file_name = command;
    if (map.contains(command)) {
      switch (map.at(command)) {
        case common::Command::kSaveFile: {
          storage_manager_.SaveFile(file_name, data);
          break;
        }

        case common::Command::kSetScreen: {
          if (const auto file = storage_manager_.LoadFile(file_name);
              !file.has_value()) {
            return;
          }
          file_processor_.process_file(file_name);
          break;
        }
        case common::Command::kDeleteFile: {
          storage_manager_.DeleteFile(file_name);
          break;
        }
        case common::Command::kNone: {
          std::cout << command_type << " " << file_name << std::endl;
          break;
        }
        default: {
          std::cout << "Unknown command: " << command << std::endl;
          break;
        }
      }
    }
  }
}
}  // namespace screen_controller
