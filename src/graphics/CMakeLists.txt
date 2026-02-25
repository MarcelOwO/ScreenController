add_library(Renderer STATIC graphics_renderer.cpp shader/shader.cpp)

target_include_directories(
  Renderer
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.)

target_link_libraries(Renderer PUBLIC common Logger glad glm::glm)
