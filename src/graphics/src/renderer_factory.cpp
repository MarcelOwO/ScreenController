

#include <graphics/renderer.hpp>

#include <memory>

#include "graphics_renderer.hpp"

namespace screen_controller {

std::unique_ptr<IRenderer> RendererFactory::Create(ILogger& logger, ProcLoader dloadproc,
                                                   int window_width, int window_height) {
  auto renderer = GraphicsRenderer::Create(logger, dloadproc, window_width, window_height);

  if (!renderer) {
    logger.LogError("Factory: Failed to create the graphics renderer");
    throw std::runtime_error("Factory: Failed to create renderer");
  }

  return std::move(renderer.value());
}

}  // namespace screen_controller
