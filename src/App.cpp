//
// Created by marce on 4/2/2025.
//

#include "App.h"

#include <iostream>

App::App()
{
    window = std::make_unique<WindowManager>();
    renderer = std::make_unique<GraphicsRenderer>();
    bluetooth = std::make_unique<BluetoothManager>();
}

App::~App()
{
    cleanup();
}

void App::init() const
{
    try
    {
        bluetooth->init();
        window->init();
        renderer->init(std::bit_cast<GLADloadproc>(window->addressPointer()), window->getWidth(), window->getHeight());
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return;
    }
}

void App::run()
{
    while (!window->shouldClose())
    {
        window->Update([this] { renderer->render(); });
        window->pollEvents();
    }
}

void App::cleanup()
{
    renderer->cleanup();
    window->cleanup();
}
