add_library(Window STATIC window_manager.cpp window_manager.h)

target_include_directories(Window
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}  # This line is crucial
)

target_link_libraries(Window PUBLIC glfw)



