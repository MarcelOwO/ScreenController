add_subdirectory(window_manager)
add_subdirectory(graphics_renderer)
add_subdirectory(bluetooth_manager)

add_library(App STATIC App.cpp App.h)

target_link_libraries(App PRIVATE Window Renderer Bluetooth)