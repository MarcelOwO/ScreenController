//
// Created by marce on 4/2/2025.
//

#include "app.h"

#include <ng-log/logging.h>


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
  if (!updater_.CheckForUpdates()) {
    LOG(INFO) << "No updates available";
  };

  CHECK(storage_manager_.Init()) << "Failed to initialize storage manager";
  CHECK(bluetooth_manager_.init()) << "Failed to initialize bluetooth manager";
  CHECK(file_processor_.init()) << "Failed to initialize file processor";

  bluetooth_manager_.on_packet_received(
      [this](const common::BluetoothPacket& packet) {
        LOG(INFO) << "Received Bluetooth packet: " << packet.name;
        (void)command_queue_.emplace(packet);
      });

  CHECK(window_manager_.init()) << "Failed to initialize window manager";

  renderer_.init(std::bit_cast<GLADloadproc>(window_manager_.address_pointer()),
                 window_manager_.get_width(), window_manager_.get_height());

  if (!load_image("startup_files/sona.png", true)) {
    LOG(ERROR) << "Failed to load startup image";
  }

  running_ = true;

  return true;
}
bool App::process_command(const common::BluetoothPacket& packet) {
  LOG(INFO) << "Received packet: " << packet.name;

  switch (packet.type) {
    case 1: {  // command packet

      const auto& command = packet.name;
      const auto it = command.find(':');

      if (it == std::string::npos) {
        LOG(ERROR) << "Invalid command format: " << command;
        return false;
      }

      const auto type = command.substr(0, it);
      const auto name = command.substr(it + 1, command.size());

      if (type == "Select") {
        LOG(INFO) << "Select command received: " << name;
        if (!load_image(name, false)) {
          LOG(ERROR) << "Failed to load image: " << name;
        }
        return true;
      }
      if (type == "Delete") {
        LOG(INFO) << "Delete command received: " << name;
        if (!storage_manager_.DeleteFile(name)) {
          PLOG(WARNING) << "Failed to delete file: " << name;
        }
        return true;
      }
      if (type == "Rotate") {
        LOG(INFO) << "Rotate command received:";
        renderer_.rotate();
        return true;
      }
      LOG(ERROR) << "Unknown command type: " << type;
      return false;
    }
    case 0: {  // file packet
      if (!storage_manager_.SaveFile(packet.name, packet.data)) {
        LOG(ERROR) << "Failed to save file: " << packet.name;
      }
      return true;
    }
    default: {
      LOG(ERROR) << "Unknown packet type: " << static_cast<int>(packet.type);
      return false;
    }
  }
  return false;
}
bool App::load_image(const std::string_view name, const bool is_asset) {
  const auto path = is_asset ? storage_manager_.GetResourcePath(name)
                             : storage_manager_.GetUserFilePath(name);

  if (!std::filesystem::exists(path)) {
    LOG(ERROR) << "File does not exist: " << path.string();
    return false;
  }

  if (!file_processor_.process_file(path.c_str())) {
    LOG(ERROR) << "Failed to process file: " << path.string();
    return false;
  }

  process_frame();

  return true;
}

void App::process_frame() {
  const auto frame = file_processor_.get_processed_data();

  if (!frame.has_value()) {
    renderer_.set_fallback_texture();
  }

  renderer_.update_ratio(frame.value()->width, frame.value()->height);

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

void App::handle_commands(const std::stop_token& stop_token) {
  while (!stop_token.stop_requested()) {
    bluetooth_manager_.run();
    std::unique_lock lock(queue_mutex_);
    queue_condition_.wait(lock, [this] { return !command_queue_.empty(); });
    if (auto packet = command_queue_.front(); !process_command(packet)) {
      LOG(ERROR) << "Failed to process command: " << packet.name;
    }
    command_queue_.pop();
    lock.unlock();
  }
}

void App::run() {
  std::jthread command_thread(&App::handle_commands, this);
  command_thread_ = std::move(command_thread);
  render_loop();
}

}  // namespace screen_controller