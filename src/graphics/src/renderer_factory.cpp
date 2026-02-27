

#include <graphics/renderer.h>

#include <memory>

#include "graphics_renderer.h"

namespace screen_controller {

std::unique_ptr<IRenderer> RendererFactory::Create(ILogger& logger) {
  return std::make_unique<GraphicsRenderer>(logger);
}

}  // namespace screen_controller
