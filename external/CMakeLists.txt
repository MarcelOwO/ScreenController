option(GLFW_INSTALL_EXAMPLES OFF)
option(GLFW_INSTALL_TESTS OFF)
option(GLFW_INSTALL_DOCS OFF)
option(GLFW_INSTALL OFF)

set(ZSTD_BUILD_STATIC ON)
set(ZSTD_BUILD_SHARED OFF)

add_subdirectory(glfw)
add_subdirectory(glad)
add_subdirectory(sdbus-cpp)
add_subdirectory(zstd/build/cmake)
add_subdirectory(glm)

add_library(stb STATIC stb.cpp)
target_include_directories(stb PUBLIC $CMAKE_CURRENT_SOURCE_DIR/.)
