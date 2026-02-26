#ifndef RENDERER_H
#define RENDERER_H

namespace screen_controller {

class IRenderer {
 public:
  virtual ~IRenderer() = 0;
  virtual std::expected<void, common::ErrorEnum> init(GLADloadproc dloadproc,
                                                      int window_width,
                                                      int window_height) = 0;
  virtual void set_texture(const common::FrameData* data) = 0;
  virtual void set_fallback_texture() const = 0;
  virtual void update_ratio(int width, int height) const = 0;
  virtual void rotate() const = 0;
  virtual void render() const = 0;
};

class RendererFactory {
 public:
  static std::unique_ptr<IRenderer> Create(ILogger& logger);
};

}  // namespace screen_controller

#endif
