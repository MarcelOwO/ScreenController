//
// Created by marce on 4/2/2025.
//

#include "app.h"

#include <logging/logger.h>

#include <thread>

#include "graphics/renderer.h"

namespace screen_controller {

App::App()
    : is_running_(true),
      logger_(LoggerFactory::Create()),
      settings_{std::make_shared<AppSettings>(settings)},
      window_manager_(WindowFactory::Create(
          *logger_, [this]() { this->is_running_ = false; })),
      renderer_(RendererFactory::Create(*logger_)),
      bluetooth_manager_(BluetoothFactory::Create(
          *logger_, this->settings,
          [this](const common::BluetoothPacket& packet) {
            logger_->LogInfo("Received Bluetooth packet: " + packet.name);
            (void)command_queue_.emplace(packet);
          })),
      storage_manager_(StorageFactory::Create(*logger_)),
      file_processor_(ProcessorFactory::Create(*logger_)) {
  logger_->LogInfo("Creating app");

  if (const auto res = storage_manager_->Init(); !res.has_value()) {
    logger_->LogError("Failed to initialize storage manager");
    return;
  }

  const auto res = renderer_->init(window_manager_->get_proc_address(),
                                   window_manager_->get_width(),
                                   window_manager_->get_height());

  if (!res) {
    logger_->LogError("Failed to initialize renderer");
  }

  if (!load_image("startup_files/eevee.gif", true)) {
    logger_->LogError("Failed to load startup image");
  }

  is_running_ = true;
}

bool App::process_command(const common::BluetoothPacket& packet) {
  logger_->LogInfo("Received packet: " + packet.name);

  switch (packet.type) {
    case 1: {
      const auto& command = packet.name;
      const auto it = command.find(':');

      if (it == std::string::npos) {
        logger_->LogError("Invalid command format: " + command);
        return false;
      }

      const auto type = command.substr(0, it);
      const auto name = command.substr(it + 1, command.size());

      if (type == "Select") {
        logger_->LogInfo("Select command received: " + name);
        if (!load_image(name, false)) {
          logger_->LogError("Failed to load image: " + name);
        }
        return true;
      }
      if (type == "Delete") {
        logger_->LogInfo("Delete command received: " + name);
        if (!storage_manager_->DeleteFile(name)) {
          logger_->LogError("Failed to delete file: " + name);
        }
        return true;
      }
      if (type == "Rotate") {
        logger_->LogInfo("Rotate command received:");
        renderer_->rotate();
        return true;
      }
      logger_->LogError("Unknown command type: " + type);
      return false;
    }
    case 0: {  // file packet
      if (!storage_manager_->SaveFile(packet.name, packet.data)) {
        logger_->LogError("Failed to save file: " + packet.name);
      }
      return true;
    }
    default: {
      logger_->LogError("Unknown packet type: " + std::to_string(packet.type));
      return false;
    }
  }
}

bool App::load_image(const std::string_view name, const bool is_asset) {
  const auto path = is_asset ? storage_manager_->GetResourcePath(name)
                             : storage_manager_->GetUserFilePath(name);

  if (!std::filesystem::exists(path)) {
    logger_->LogError("File does not exist: " + path.string());
    return false;
  }

  if (!file_processor_->process_file(path.c_str())) {
    logger_->LogError("Failed to process file: " + path.string());
    return false;
  }

  process_frame();

  return true;
}

void App::process_frame() {
  const auto frame = file_processor_->get_processed_data();

  if (!frame.has_value()) {
    renderer_->set_fallback_texture();
  }

  renderer_->update_ratio(frame.value()->width, frame.value()->height);

  renderer_->set_texture(frame.value().get());
}

void App::handle_commands(const std::stop_token& stop_token) {
  while (!stop_token.stop_requested()) {
    std::unique_lock lock(queue_mutex_);
    queue_condition_.wait(lock, [this] { return !command_queue_.empty(); });
    if (auto packet = command_queue_.front(); !process_command(packet)) {
      logger_->LogError("Failed to process command: " + packet.name);
    }
    command_queue_.pop();
    lock.unlock();
  }
}

void App::run() {
  std::jthread command_thread(&App::handle_commands, this);
  command_thread_ = std::move(command_thread);
  render_loop();
  if (command_thread_.joinable()) {
    command_thread_.join();
  }
}

void App::render_loop() {
  while (is_running_) {
    process_frame();
    window_manager_->update([this] { renderer_->render(); });
    window_manager_->poll_events();
    std::this_thread::sleep_for(std::chrono::milliseconds{16});

    if (window_manager_->should_close()) {
      is_running_ = false;
    }
  }
}

App::~App() {
  logger_->LogInfo("Cleaning up App class");
  queue_condition_.notify_all();
}
}  // namespace screen_controller
