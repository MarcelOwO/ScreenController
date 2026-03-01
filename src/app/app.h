//
// Created by marce on 4/2/2025.
//

#ifndef APP_H
#define APP_H

#include <bluetooth_packet.h>
#include <bt/manager.h>
#include <graphics/renderer.h>
#include <logging/logger.h>
#include <processor/file_processor.h>
#include <storage/storage_manager.h>
#include <window_manager/window_manager.h>

#include <condition_variable>
#include <queue>
#include <thread>

#include "app_settings.h"

namespace screen_controller {
class App {
 public:
  App();
  ~App();

  bool process_command(const common::BluetoothPacket& packet);
  void run();

  AppSettings settings;

 private:
  bool is_running_;
  std::jthread command_thread_;

  std::shared_ptr<ILogger> logger_;

  std::shared_ptr<AppSettings> settings_;

  std::unique_ptr<IWindowManager> window_manager_;
  std::unique_ptr<IRenderer> renderer_;
  std::unique_ptr<IStorageManager> storage_manager_;
  std::unique_ptr<IFileProcessor> file_processor_;
  std::unique_ptr<IBluetoothManager> bluetooth_manager_;

  std::queue<common::BluetoothPacket> command_queue_;
  std::mutex queue_mutex_;
  std::condition_variable queue_condition_;

  bool load_image(std::string_view name, bool is_asset);
  void process_frame();
  void render_loop();
  void handle_commands(const std::stop_token& stop_token);
};
}  // namespace screen_controller
#endif  // APP_H
