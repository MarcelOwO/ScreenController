//
// Created by marce on 4/2/2025.
//

#ifndef APP_H
#define APP_H

#include <graphics_renderer/graphics_renderer.h>
#include <logging/logger.h>
#include <processor/file_processor.h>
#include <storage_manager/storage_manager.h>
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
  bool running_;
  std::jthread command_thread_;

  std::shared_ptr<Logger> logger_;
  std::shared_ptr<AppSettings> settings_;

  WindowManager window_manager_;
  GraphicsRenderer renderer_;
  StorageManager storage_manager_;
  FileProcessor file_processor_;

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
