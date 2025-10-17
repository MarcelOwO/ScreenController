
add_library(Renderer STATIC
        graphics_renderer.cpp
        shader/shader.cpp
)

target_include_directories(Renderer
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.
)

target_link_libraries(Renderer
        PUBLIC common  Logger
        PRIVATE glad glm::glm
)

add_custom_command(
        TARGET Renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets/shader_files"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/graphics_renderer/shader_files"
        "${CMAKE_BINARY_DIR}/assets/shader_files"
)