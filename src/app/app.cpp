//
// Created by marce on 4/2/2025.
//

#include "app.hpp"

#include <iostream>
#include <thread>

#include <graphics/renderer.hpp>
#include <logging/logger.hpp>

namespace screen_controller {

App::App() try
    : is_running_(false),
      logger_(LoggerFactory::Create()),
      settings_(std::make_unique<AppSettings>()),
      renderer_(RendererFactory::Create(*logger_, window_manager_->get_proc_address(),
                                        window_manager_->get_width(),
                                        window_manager_->get_height())),
      window_manager_(WindowFactory::Create(*logger_, [this] { is_running_ = false; })),
      bluetooth_manager_(BluetoothFactory::Create(*logger_, *settings_,
                                                  [this](const auto& packet) {
                                                    // std::lock_guard lock(queue_mutex_);
                                                    // command_queue_.push(packet);
                                                    // queue_condition_.notify_one();
                                                  })),
      storage_manager_(StorageFactory::Create(*logger_)),
      event_manager_(EventFactory::Create(*logger_, *settings_)),
      file_processor_(ProcessorFactory::Create(*logger_)) {
  logger_->LogInfo("Initializing App Subsystems...");

  if (LoadImage("startup_files/eevee.gif", true)) {
    logger_->LogInfo("Startup image loaded.");
  }

  is_running_ = true;
} catch (std::exception& e) {
  std::cerr << "Failed to create submodules" << e.what() << '\n';
}

void App::AdjustSettings(const std::function<void(AppSettings&)>& update_settings) {
  update_settings(*settings_);
}

bool App::ProcessCommand(const BluetoothPacket& packet) {
  logger_->LogInfo("Received packet: " + packet.name);

  switch (packet.type) {
    case 1: {
      const auto& command = packet.name;
      const auto kIt = command.find(':');

      if (kIt == std::string::npos) {
        logger_->LogError("Invalid command format: " + command);
        return false;
      }

      const auto kType = command.substr(0, kIt);
      const auto kName = command.substr(kIt + 1, command.size());

      if (kType == "Select") {
        logger_->LogInfo("Select command received: " + kName);
        if (!LoadImage(kName, false)) {
          logger_->LogError("Failed to load image: " + kName);
        }
        return true;
      }
      if (kType == "Delete") {
        logger_->LogInfo("Delete command received: " + kName);
        if (!storage_manager_->DeleteFile(kName)) {
          logger_->LogError("Failed to delete file: " + kName);
        }
        return true;
      }
      if (kType == "Rotate") {
        logger_->LogInfo("Rotate command received:");
        renderer_->Rotate();
        return true;
      }
      logger_->LogError("Unknown command type: " + kType);
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

bool App::LoadImage(std::string_view k_name, bool k_is_asset) {
  const auto kPath = k_is_asset ? storage_manager_->GetResourcePath(k_name)
                                : storage_manager_->GetUserFilePath(k_name);

  if (!std::filesystem::exists(kPath)) {
    logger_->LogError("File does not exist: " + kPath.string());
    return false;
  }

  if (!file_processor_->process_file(kPath.c_str())) {
    logger_->LogError("Failed to process file: " + kPath.string());
    return false;
  }

  ProcessFrame();

  return true;
}

void App::ProcessFrame() {
  const auto kFrame = file_processor_->get_processed_data();

  if (!kFrame.has_value()) {
    renderer_->SetFallbackTexture();
  }

  renderer_->UpdateRatio(kFrame.value()->width, kFrame.value()->height);

  renderer_->SetTexture(kFrame.value().get());
}

void App::HandleCommands(const std::stop_token& stop_token) {
  while (!stop_token.stop_requested()) {
    // std::unique_lock lock(queue_mutex_);
    // queue_condition_.wait(lock, [this] { return !command_queue_.empty(); });
    // if (auto packet = command_queue_.front(); !ProcessCommand(packet)) {
    //   logger_->LogError("Failed to process command: " + packet.name);
    // }
    // command_queue_.pop();
    // lock.unlock();
  }
}

void App::Run() {
  // std::jthread command_thread(&App::HandleCommands, this);
  // command_thread_ = std::move(command_thread);
  RenderLoop();
  // if (command_thread_.joinable()) {
  //   command_thread_.join();
  // }
}

void App::RenderLoop() {
  while (is_running_) {
    ProcessFrame();
    window_manager_->update([this] { renderer_->Render(); });
    window_manager_->poll_events();
    std::this_thread::sleep_for(std::chrono::milliseconds{16});

    if (window_manager_->should_close()) {
      is_running_ = false;
    }
  }
}

App::~App() {
  logger_->LogInfo("Cleaning up App class");
  //queue_condition_.notify_all();
}
}  // namespace screen_controller
