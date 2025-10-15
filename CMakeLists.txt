cmake_minimum_required(VERSION 3.25)
project(ScreenController VERSION 1.0.0 )

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_subdirectory(external)
add_subdirectory(source)

add_executable(ScreenController source/main.cpp)

target_link_libraries(ScreenController
PRIVATE
App common ng-log)


add_custom_command(
        TARGET ScreenController POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "${CMAKE_BINARY_DIR}/assets"
)
