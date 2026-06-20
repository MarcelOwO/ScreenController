//
// Created by marce on 4/2/2025.
//

#include "app.hpp"

#include <chrono>
#include <iostream>
#include <thread>

namespace screen_controller {

App::App() try
    : is_running_(false),
      logger_(LoggerFactory::Create()),
      settings_(std::make_unique<AppSettings>()),
      window_manager_(WindowFactory::Create(*logger_, [this] { is_running_ = false; })),
      renderer_(RendererFactory::Create(*logger_, window_manager_->get_proc_address(),
                                        window_manager_->get_width(),
                                        window_manager_->get_height())),
      storage_manager_(StorageFactory::Create(*logger_)),
      event_manager_(EventFactory::Create(*logger_, *settings_)),
      file_processor_(ProcessorFactory::Create(*logger_)),
      bluetooth_manager_(BluetoothFactory::Create(*logger_, *settings_, *event_manager_)) {
  event_manager_->Subscribe<CommandReceivedEvent>(
      [this](const CommandReceivedEvent& e) { OnCommandReceived(e); });

  event_manager_->Subscribe<FileReceivedEvent>(
      [this](const FileReceivedEvent& e) { OnFileReceived(e); });

  logger_->LogInfo("App subsystems initialized");

  if (LoadImage("startup_files/eevee.gif", true)) {
    logger_->LogInfo("Startup image loaded");
  }

  is_running_ = true;
} catch (const std::exception& e) {
  std::cerr << "Failed to create App: " << e.what() << '\n';
}

void App::AdjustSettings(const std::function<void(AppSettings&)>& update_settings) {
  update_settings(*settings_);
}

void App::OnCommandReceived(const CommandReceivedEvent& event) {
  const auto& command = event.command;
  logger_->LogFmt(LogLevel::INFO, "Command received: {}", command);

  if (command == "Rotate") {
    renderer_->Rotate();
    return;
  }

  const auto kSep = command.find(':');
  if (kSep == std::string::npos) {
    logger_->LogFmt(LogLevel::ERROR, "Unknown command: {}", command);
    return;
  }

  const auto kType = command.substr(0, kSep);
  const auto kName = command.substr(kSep + 1);

  if (kType == "Select") {
    if (!LoadImage(kName, false)) {
      logger_->LogFmt(LogLevel::ERROR, "Failed to load image: {}", kName);
    }
  } else if (kType == "Delete") {
    if (!storage_manager_->DeleteFile(kName)) {
      logger_->LogFmt(LogLevel::ERROR, "Failed to delete file: {}", kName);
    }
  } else {
    logger_->LogFmt(LogLevel::ERROR, "Unknown command type: {}", kType);
  }
}

void App::OnFileReceived(const FileReceivedEvent& event) {
  logger_->LogFmt(LogLevel::INFO, "File received: {} ({} bytes)", event.filename,
                  event.data.size());

  if (!storage_manager_->SaveFile(event.filename, event.data)) {
    logger_->LogFmt(LogLevel::ERROR, "Failed to save file: {}", event.filename);
  }
}

bool App::LoadImage(std::string_view name, bool is_asset) {
  const auto kPath = is_asset ? storage_manager_->GetResourcePath(name)
                              : storage_manager_->GetUserFilePath(name);

  if (!std::filesystem::exists(kPath)) {
    logger_->LogFmt(LogLevel::ERROR, "File does not exist: {}", kPath.string());
    return false;
  }

  if (!file_processor_->ProcessFile(kPath.string())) {
    logger_->LogFmt(LogLevel::ERROR, "Failed to process file: {}", kPath.string());
    return false;
  }

  ProcessFrame();
  return true;
}

void App::ProcessFrame() {
  auto kFrame = file_processor_->GetProcessedData();

  if (!kFrame.has_value()) {
    renderer_->SetFallbackTexture();
    return;
  }

  renderer_->UpdateRatio(kFrame.value()->width, kFrame.value()->height);
  renderer_->SetTexture(kFrame.value().get());
}

void App::Run() {
  RenderLoop();
}

void App::RenderLoop() {
  while (is_running_) {
    bluetooth_manager_->Poll();
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
  logger_->LogInfo("Shutting down App");
}

}  // namespace screen_controller
