//
// Created by marce on 4/2/2025.
//

#ifndef APP_H
#define APP_H

#include <memory>

#include <../bluetooth_manager/bluetooth_manager.h>
#include <../graphics_renderer/graphics_renderer.h>
#include <../storage_manager/storage_manager.h>
#include <../window_manager/window_manager.h>
#include <../file_processor/file_processor.h>
#include <../common/command.h>

class App {
public:
    App();
    ~App();

    void init();
    void run();
    void cleanup();

private:
    WindowManager window_manager_;
    GraphicsRenderer renderer_;
    BluetoothManager bluetooth_manager_;
    StorageManager storage_manager_;
    FileProcessor file_processor_;
};

#endif //APP_H
