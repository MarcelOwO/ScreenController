//
// Created by marce on 4/2/2025.
//

#include "app.hpp"

#include <charconv>
#include <chrono>
#include <format>
#include <thread>

namespace screen_controller {

App::App()
    : is_running_(false),
      logger_(LoggerFactory::Create()),
      settings_(),
      window_manager_(WindowFactory::Create(*logger_, [this] { is_running_ = false; })),
      renderer_(RendererFactory::Create(*logger_, window_manager_->GetProcAddress(),
                                        window_manager_->GetWidth(), window_manager_->GetHeight())),
      storage_manager_(StorageFactory::Create(*logger_)),
      event_manager_(EventFactory::Create(*logger_, settings_)),
      file_processor_(ProcessorFactory::Create(*logger_)),
      bluetooth_manager_(BluetoothFactory::Create(*logger_, settings_, *event_manager_)) {
  event_manager_->Subscribe<CommandReceivedEvent>(
      [this](const CommandReceivedEvent& event) { OnCommandReceived(event); });

  event_manager_->Subscribe<FileReceivedEvent>(
      [this](const FileReceivedEvent& event) { OnFileReceived(event); });

  logger_->LogInfo("App subsystems initialized");

  renderer_->SetFallbackTexture();

  if (LoadImage("startup_files/eevee.gif", true)) {
    logger_->LogInfo("Startup image loaded");
  }

  is_running_ = true;
}

void App::OnCommandReceived(const CommandReceivedEvent& event) {
  const auto& command = event.command;
  logger_->LogFmt(LogLevel::INFO, "Command received: {}", command);

  if (command == "Rotate") {
    renderer_->Rotate();
    SendAck("Rotate");
    return;
  }

  if (command == "GetFiles") {
    SendFileList();
    return;
  }

  if (command == "GetStatus") {
    SendStatus();
    return;
  }

  if (command == "ScreenOff") {
    display_enabled_ = false;
    renderer_->SetDisplayEnabled(false);
    SendAck("ScreenOff");
    return;
  }

  if (command == "ScreenOn") {
    display_enabled_ = true;
    renderer_->SetDisplayEnabled(true);
    SendAck("ScreenOn");
    return;
  }

  const auto kSep = command.find(':');
  if (kSep == std::string::npos) {
    logger_->LogFmt(LogLevel::ERROR, "Unknown command: {}", command);
    SendError(0x04, "unknown command");
    return;
  }

  const auto kType = command.substr(0, kSep);
  const auto kName = command.substr(kSep + 1);

  if (kType == "Select") {
    if (!LoadImage(kName, false)) {
      logger_->LogFmt(LogLevel::ERROR, "Failed to load image: {}", kName);
      SendError(0x07, "file could not be selected");
    } else {
      SendAck("Select");
    }
  } else if (kType == "Delete") {
    if (!storage_manager_->DeleteFile(kName)) {
      logger_->LogFmt(LogLevel::ERROR, "Failed to delete file: {}", kName);
      SendError(0x06, "file could not be deleted");
    } else {
      SendAck("Delete");
    }
  } else if (kType == "SetBrightness") {
    int value = 0;
    const auto [end, error] = std::from_chars(kName.data(), kName.data() + kName.size(), value);
    if (error != std::errc{} || end != kName.data() + kName.size() || value < 0 || value > 100) {
      SendError(0x08, "brightness must be an integer from 0 to 100");
      return;
    }
    brightness_percent_ = value;
    renderer_->SetBrightness(static_cast<float>(value) / 100.0F);
    SendAck("SetBrightness");
  } else {
    logger_->LogFmt(LogLevel::ERROR, "Unknown command type: {}", kType);
    SendError(0x04, "unknown command");
  }
}

void App::SendFileList() {
  const auto kFiles = storage_manager_->ListFiles();

  // Build newline-separated payload.
  std::string payload;
  for (const auto& name : kFiles) {
    payload += name;
    payload += '\n';
  }

  logger_->LogFmt(LogLevel::INFO, "Sending file list ({} files)", kFiles.size());

  const auto kSpan = std::span<const std::byte>(reinterpret_cast<const std::byte*>(payload.data()),
                                                payload.size());

  constexpr uint8_t kTypeFileList = 0xC1;
  bluetooth_manager_->SendPacket(kTypeFileList, "files", kSpan);
}

void App::OnFileReceived(const FileReceivedEvent& event) {
  logger_->LogFmt(LogLevel::INFO, "File received: {} ({} bytes)", event.filename,
                  event.data.size());

  if (!storage_manager_->SaveFile(event.filename, event.data)) {
    logger_->LogFmt(LogLevel::ERROR, "Failed to save file: {}", event.filename);
    SendError(0x06, "file could not be saved");
    return;
  }
  SendAck("Upload");
}

bool App::LoadImage(std::string_view name, bool is_asset) {
  const auto kPath =
      is_asset ? storage_manager_->GetResourcePath(name) : storage_manager_->GetUserFilePath(name);

  if (kPath.empty() || !std::filesystem::exists(kPath)) {
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
  const auto kFrame = file_processor_->GetProcessedData();

  if (!kFrame.has_value()) {
    return;
  }

  renderer_->UpdateRatio(kFrame.value()->width, kFrame.value()->height);
  renderer_->SetTexture(kFrame.value().get());
}

void App::SendStatus() {
  const std::string payload = std::format(R"({{"brightness":{},"displayEnabled":{}}})",
                                          brightness_percent_, display_enabled_ ? "true" : "false");
  const auto bytes = std::as_bytes(std::span{payload});
  constexpr uint8_t kTypeStatus = 0xC3;
  (void) bluetooth_manager_->SendPacket(kTypeStatus, "status", bytes);
}

void App::SendAck(const std::string_view operation) {
  constexpr uint8_t kTypeAck = 0xC2;
  (void) bluetooth_manager_->SendPacket(kTypeAck, operation);
}

void App::SendError(const uint32_t code, const std::string_view message) {
  const std::string payload = std::format("ERR:{}:{}", code, message);
  const auto bytes = std::as_bytes(std::span{payload});
  constexpr uint8_t kTypeError = 0xCF;
  (void) bluetooth_manager_->SendPacket(kTypeError, "error", bytes);
}

void App::Run() {
  RenderLoop();
}

void App::RenderLoop() {
  // ~60 fps target.
  constexpr auto kFrameInterval = std::chrono::milliseconds{16};

  while (is_running_) {
    bluetooth_manager_->Poll();
    ProcessFrame();
    window_manager_->Update([this] { renderer_->Render(); });
    window_manager_->PollEvents();
    std::this_thread::sleep_for(kFrameInterval);

    if (window_manager_->ShouldClose()) {
      is_running_ = false;
    }
  }
}

App::~App() {
  logger_->LogInfo("Shutting down App");
}

}  // namespace screen_controller
