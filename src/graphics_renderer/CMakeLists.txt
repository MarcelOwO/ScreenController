add_library(Renderer STATIC
        graphics_renderer.cpp
        graphics_renderer.h
)
target_include_directories(Renderer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}  # This line is crucial
)

target_link_libraries(Renderer PUBLIC glad)
