cmake_minimum_required(VERSION 3.25)
project(ScreenController)

set(CMAKE_CXX_STANDARD 26)

add_subdirectory(src/common)
add_subdirectory(src/app)
add_subdirectory(src/window_manager)
add_subdirectory(src/graphics_renderer)
add_subdirectory(src/bluetooth_manager)
add_subdirectory(src/storage_manager)
add_subdirectory(src/file_processor)

add_subdirectory(external)

add_executable(ScreenController src/main.cpp)

target_link_libraries(ScreenController PRIVATE App common)
