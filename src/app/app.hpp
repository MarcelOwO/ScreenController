//
// Created by marce on 4/2/2025.
//

#pragma once

#include <bt/manager.hpp>
#include <events/events.hpp>
#include <graphics/renderer.hpp>
#include <logging/logger.hpp>
#include <models/bluetooth_packet.hpp>
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

  void AdjustSettings(const std::function<void(AppSettings&)>& update_settings);

  bool ProcessCommand(const BluetoothPacket& packet);

  void Run();

private:
  bool is_running_;

  std::unique_ptr<ILogger> logger_;
  std::unique_ptr<AppSettings> settings_;

  std::unique_ptr<IWindowManager> window_manager_;
  std::unique_ptr<IRenderer> renderer_;
  std::unique_ptr<IStorageManager> storage_manager_;
  std::unique_ptr<IEventManager> event_manager_;
  std::unique_ptr<IFileProcessor> file_processor_;
  std::unique_ptr<IBluetoothManager> bluetooth_manager_;

  bool LoadImage(std::string_view k_name, bool k_is_asset);
  void ProcessFrame();
  void RenderLoop();
};
}  // namespace screen_controller
