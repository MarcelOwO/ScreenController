

#include <graphics/renderer.h>

#include <memory>

#include "graphics_renderer.h"

namespace screen_controller {

std::unique_ptr<IRenderer> RendererFactory::Create() {
  return std::make_unique<GraphicsRenderer>();
}

}  // namespace screen_controller
