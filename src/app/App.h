//
// Created by marce on 4/2/2025.
//

#ifndef APP_H
#define APP_H

#include <memory>

#include <../bluetooth_manager/bluetooth_manager.h>
#include <../graphics_renderer/graphics_renderer.h>
#include <../window_manager/window_manager.h>

class App {
public:
    App();
    ~App();

    void init() const;
    void run() const;
    void cleanup() const;

private:
    std::unique_ptr<WindowManager> window_;
    std::unique_ptr<GraphicsRenderer> renderer_;
    std::unique_ptr<BluetoothManager> bluetooth_;
};


#endif //APP_H
