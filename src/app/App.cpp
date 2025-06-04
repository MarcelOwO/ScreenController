//
// Created by marce on 4/2/2025.
//

#include "App.h"

#include <ng-log/logging.h>

#include <iostream>

#include "command.h"

namespace screen_controller {

App::App() : running_(false) { LOG(INFO) << "Creating app"; }

App::~App() {
  LOG(INFO) << "Cleaning up App class";
  running_ = false;
  queue_condition_.notify_all();
  if (command_thread_.joinable()) {
    command_thread_.join();
  }
}

bool App::init() {
  PCHECK(storage_manager_.Init()) << "Failed to initialize storage manager";
  PCHECK(bluetooth_manager_.init()) << "Failed to initialize bluetooth manager";
  PCHECK(file_processor_.init()) << "Failed to initialize file processor";

  bluetooth_manager_.on_blutooth_packet_received(
      [this](common::BluetoothPacket packet) {
        switch (packet.type) {
          case 1: {  // command packet
            const auto command = packet.name;
            const auto it = command.find(':');
            if (it == std::string::npos) {
              LOG(ERROR) << "Invalid command format: " << command;
              return;
            }
            const auto type = command.substr(0, it);
            const auto name = command.substr(it + 1, command.size());

            if (type == "Select") {
              LOG(INFO) << "Select command received: " << name;
              if (!load_image(name, false)) {
              }
              break;
            }
            if (type == "Delete") {
              LOG(INFO) << "Delete command received: " << name;
              if (!storage_manager_.DeleteFile(name)) {
                PLOG(WARNING) << "Failed to delete file: " << name;
              }
              break;
            }

            break;
          }
          case 0: {  // file packet
            if (!storage_manager_.SaveFile(packet.name, packet.data)) {
              PLOG(WARNING) << "Failed to save file: " << packet.name;
            }
            break;
          }
          default: {
            LOG(ERROR) << "Unknown packet type: "
                       << static_cast<int>(packet.type);
          }
        }
      });

  PCHECK(window_manager_.init()) << "Failed to initialize window manager";

  renderer_.init(std::bit_cast<GLADloadproc>(window_manager_.address_pointer()),
                 window_manager_.get_width(), window_manager_.get_height());

  PCHECK(load_image("sona.png", true)) << "Failed to load startup image";

  running_ = true;

  return true;
}
bool App::load_image(std::string_view name, bool is_asset) {
  const auto path = is_asset ? storage_manager_.GetResourcePath(name)
                             : storage_manager_.GetUserFilePath(name);
  if (!std::filesystem::exists(path)) {
    PLOG(WARNING) << "File does not exist: " << path.string();
    return false;
  }

  PCHECK(file_processor_.process_file(path.c_str()))
      << "Failed to load startup image";

  process_frame();

  return true;
}

void App::process_frame() {
  const auto frame = file_processor_.get_processed_data();

  if (!frame.has_value()) {
    LOG(WARNING) << "No frame data available";
    return;
  }

  renderer_.set_texture(frame.value().get());
}

void App::render_loop() {
  while (!window_manager_.should_close()) {
    bluetooth_manager_.run();
    process_frame();
    window_manager_.update([this] { renderer_.render(); });
    window_manager_.poll_events();
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void App::handle_commands() {
  while (true) {
    bluetooth_manager_.run();
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
          PLOG(ERROR) << "Unknown command: " << command;
          break;
        }
      }
    }
  }
}
}  // namespace screen_controller
