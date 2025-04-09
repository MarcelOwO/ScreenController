add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw ${CMAKE_BINARY_DIR}/external/glfw)

find_package(GLFW CONFIG REQUIRED)

add_library(Window STATIC window_manager.cpp window_manager.h)

target_link_libraries(Window PRIVATE GLFW)



