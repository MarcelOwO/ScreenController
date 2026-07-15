//
// Created by marce on 4/2/2025.
//

#pragma once

#include <bt/manager.hpp>
#include <events/events.hpp>
#include <graphics/renderer.hpp>
#include <logging/logger.hpp>
#include <processor/file_processor.hpp>
#include <storage/storage_manager.hpp>
#include <window_manager/window_manager.hpp>

#include <filesystem>

#include "models/app_settings.hpp"

namespace screen_controller {

class App {
public:
  App();
  ~App();

  void Run();

private:
  bool is_running_;

  std::unique_ptr<ILogger> logger_;
  AppSettings settings_;

  std::unique_ptr<IWindowManager> window_manager_;
  std::unique_ptr<IRenderer> renderer_;
  std::unique_ptr<IStorageManager> storage_manager_;
  std::unique_ptr<IEventManager> event_manager_;
  std::unique_ptr<IFileProcessor> file_processor_;
  std::unique_ptr<IBluetoothManager> bluetooth_manager_;

  bool LoadImage(std::string_view name, bool is_asset);
  void ProcessFrame();
  void RenderLoop();

  void OnCommandReceived(const CommandReceivedEvent& event);
  void OnFileReceived(const FileReceivedEvent& event);
  void SendFileList();
  void SendStatus();
  void SendAck(std::string_view operation);
  void SendError(uint32_t code, std::string_view message);

  int brightness_percent_{100};
  bool display_enabled_{true};
};

}  // namespace screen_controller
