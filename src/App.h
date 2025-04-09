//
// Created by marce on 4/2/2025.
//

#ifndef APP_H
#define APP_H

#include "window_manager/window_manager.h"

#include <exception>
#include <memory>
#include <stdexcept>

#include "bluetooth_manager/bluetooth_manager.h"
#include "graphics_renderer/graphics_renderer.h"

class App
{
public:
    App();
    ~App();

    void init() const;
    void run();
    void cleanup();

private:
    std::unique_ptr<WindowManager> window;
    std::unique_ptr<GraphicsRenderer> renderer;
    std::unique_ptr<BluetoothManager> bluetooth;
};


#endif //APP_H
