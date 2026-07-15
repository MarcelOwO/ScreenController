option(GLFW_INSTALL_EXAMPLES OFF)
option(GLFW_INSTALL_TESTS OFF)
option(GLFW_INSTALL_DOCS OFF)
option(GLFW_INSTALL OFF)

if(LINUX)
  # Raspberry Pi OS runs the kiosk under Xwayland/X11. Avoid requiring the full
  # Wayland protocol development stack for this dedicated deployment.
  set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW Wayland backend" FORCE)
endif()

set(ZSTD_BUILD_STATIC ON)
set(ZSTD_BUILD_SHARED OFF)
set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_BUILD_TESTS OFF)
set(SDBUSCPP_INSTALL OFF CACHE BOOL "Do not install the embedded sdbus-c++" FORCE)

add_subdirectory(glfw EXCLUDE_FROM_ALL)
add_subdirectory(glad EXCLUDE_FROM_ALL)
add_subdirectory(zstd/build/cmake EXCLUDE_FROM_ALL)
add_subdirectory(glm EXCLUDE_FROM_ALL)
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

if(LINUX)
  add_subdirectory(sdbus-cpp EXCLUDE_FROM_ALL)
endif()

add_library(stb STATIC stb.cpp)
target_include_directories(stb PUBLIC $CMAKE_CURRENT_SOURCE_DIR/.)
