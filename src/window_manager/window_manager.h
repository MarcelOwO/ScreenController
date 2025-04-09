//
// Created by marce on 4/2/2025.
//

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H


#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <functional>

class WindowManager
{
public:
    WindowManager();
    ~WindowManager();

    void init();
    void Update(const std::function<void()>& render);
    void cleanup();

    bool shouldClose();
    void pollEvents() const;

    void swapBuffers();

    int getHeight();
    int getWidth();

    GLFWglproc (* addressPointer())(const char* procname);

private:
    GLFWwindow* window;
};


#endif //WINDOW_MANAGER_H
