#include <graphics/renderer.hpp>
#include <helper/unwrap.hpp>
#include <memory>

#include "graphics_renderer.hpp"

namespace screen_controller {

std::unique_ptr<IRenderer> RendererFactory::Create(ILogger& logger, ProcLoader dloadproc,
                                                   int window_width, int window_height) {
  return Unwrap(GraphicsRenderer::Create(logger, dloadproc, window_width, window_height),
                "Failed to create graphics renderer");
}

}  // namespace screen_controller
