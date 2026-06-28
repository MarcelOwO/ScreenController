#pragma once

#include <logging/logger.hpp>
#include <models/frame_data.hpp>

namespace screen_controller {

class IRenderer {
public:
  virtual ~IRenderer() = default;

  using ProcLoader = void* (*) (const char*);

  virtual void SetTexture(const FrameData* data) = 0;
  virtual void SetFallbackTexture() const = 0;
  virtual void UpdateRatio(int width, int height) const = 0;
  virtual void Rotate() const = 0;
  virtual void Render() const = 0;
};

class RendererFactory {
public:
  using ProcLoader = void* (*) (const char*);

  static std::unique_ptr<IRenderer> Create(ILogger& logger, ProcLoader dloadproc, int window_width,
                                           int window_height);
};

}  // namespace screen_controller
