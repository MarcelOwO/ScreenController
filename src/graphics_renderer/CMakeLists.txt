add_library(Renderer STATIC
        graphics_renderer.cpp
        graphics_renderer.h
        shader/shader.cpp
        shader/shader.h
)

target_include_directories(Renderer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}  # This line is crucial
)

target_link_libraries(Renderer PUBLIC common glad ng-log)
add_custom_command(
        TARGET Renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets/shader_files"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/graphics_renderer/shader_files"
        "${CMAKE_BINARY_DIR}/assets/shader_files"
)