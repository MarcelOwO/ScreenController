//
// Created by marce on 4/2/2025.
//

#ifndef APP_H
#define APP_H

#include <../bluetooth_manager/bluetooth_manager.h>
#include <../file_processor/file_processor.h>
#include <../graphics_renderer/graphics_renderer.h>
#include <../storage_manager/storage_manager.h>
#include <../window_manager/window_manager.h>

#include <memory>
#include <queue>

namespace screen_controller {
class App {
 public:
  App();
  ~App();
  bool init();
  void run();

 private:
  bool running_;
  std::thread command_thread_;

  WindowManager window_manager_;
  GraphicsRenderer renderer_;
  BluetoothManager bluetooth_manager_;
  StorageManager storage_manager_;
  FileProcessor file_processor_;

  std::queue<std::pair<std::string, std::vector<std::byte>>> command_queue_;
  std::mutex queue_mutex_;
  std::condition_variable queue_condition_;

  bool load_startup_image();
  void process_frame();
  void render_loop();
  void handle_commands();
  void command_callback(const std::string& command,
                        const std::vector<std::byte>& data);
};
}  // namespace screen_controller
#endif  // APP_H
