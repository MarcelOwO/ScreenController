cmake_minimum_required(VERSION 3.25)
project(ScreenController)

set(CMAKE_CXX_STANDARD 26)

add_subdirectory(src)

add_executable(ScreenController src/main.cpp)

target_link_libraries(ScreenController PRIVATE App)
