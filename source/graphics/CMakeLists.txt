add_library(Renderer STATIC
  src/graphics_renderer.cpp
  src/renderer_factory.cpp
  src/shader/shader.cpp
)

target_include_directories(
  Renderer
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Renderer
  PUBLIC
  common Logger
  PRIVATE
  glad glm::glm
)
