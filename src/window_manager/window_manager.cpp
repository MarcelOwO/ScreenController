//
// Created by marce on 4/2/2025.
//

#include "window_manager.h"

#include <iostream>

WindowManager::WindowManager()
{
    window = nullptr;
}

void WindowManager::init()
{
    glfwSetErrorCallback([](int, const char* description)
    {
        std::cout << "Error: " << description << "\n";
    });


    if (!glfwInit())
    {
        return;
    }

	window = glfwCreateWindow(1920, 1080, "My Title", glfwGetPrimaryMonitor(), nullptr);

    if (!window)
    {
        return;
    }


    glfwMakeContextCurrent(window);
}

bool WindowManager::shouldClose()
{
    return glfwWindowShouldClose(window);
}

void WindowManager::pollEvents() const
{
    return glfwPollEvents();
}

void WindowManager::swapBuffers()
{
    glfwSwapBuffers(window);
}

int WindowManager::getHeight()
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    return mode->height;
}

int WindowManager::getWidth()
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    return mode->width;
}

GLFWglproc (* WindowManager::addressPointer())(const char* procname)
{
    return &glfwGetProcAddress;
}

void WindowManager::Update(const std::function<void()>& render)
{
    if (!window)
    {
        return;
    }

    render();

    glfwSwapBuffers(window);

    glfwSwapInterval(1);
}

WindowManager::~WindowManager()
{
    cleanup();
}

void WindowManager::cleanup()
{
    if (window)
    {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
