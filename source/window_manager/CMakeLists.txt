add_library(Window STATIC window_manager.cpp)

target_include_directories(Window
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Window PUBLIC Logger common
        PRIVATE glfw)



