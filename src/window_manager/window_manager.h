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
    void update(const std::function<void()>& render) const;
    void cleanup() const;

    [[nodiscard]] bool should_close() const;
    static void poll_events();

    void swap_buffers() const;

    static int get_height();
    static int get_width();

    static GLFWglproc (* address_pointer())(const char* procname);

private:
    GLFWwindow* window_;
};


#endif //WINDOW_MANAGER_H
