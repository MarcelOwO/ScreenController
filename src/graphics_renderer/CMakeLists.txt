find_package(OpenGL REQUIRED)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glad ${CMAKE_BINARY_DIR}/external/glad)

add_library(Renderer STATIC
        graphics_renderer.cpp
        graphics_renderer.h
)

target_link_libraries(Renderer PRIVATE glad OpenGL::GL)

target_include_directories(Renderer PUBLIC
        ${CMAKE_SOURCE_DIR}/external/glad>
)