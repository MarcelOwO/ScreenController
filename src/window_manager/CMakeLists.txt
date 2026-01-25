add_library(Window STATIC src/window_factory.cpp src/glfw_window.cpp)

target_include_directories(Window
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(Window PUBLIC Logger common glfw)



