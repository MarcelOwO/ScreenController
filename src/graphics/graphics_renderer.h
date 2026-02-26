//
// Created by marce on 4/2/2025.
//

#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include <glad/glad.h>
#include <graphics/renderer.h>
#include <logging/logger.h>
#include <models/frame_data.h>
#include <pixel_data.h>

#include <expected>
#include <filesystem>

#include "models/error_enum.h"
#include "shader/shader.h"

namespace screen_controller {

class GraphicsRenderer : public IRenderer {
 public:
  explicit GraphicsRenderer(ILogger& logger);
  ~GraphicsRenderer();

  struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  std::expected<void, common::ErrorEnum> init(GLADloadproc dloadproc,
                                              int window_width,
                                              int window_height);

  void set_texture(const common::FrameData* data);
  void set_fallback_texture() const;
  void update_ratio(int width, int height) const;

  void rotate() const;

  void render() const;

 private:
  ILogger& logger_;
  Shader shader_;

  GLuint texture_;

  GLuint vao_;
  GLuint vbo_;

  const std::filesystem::path vertex_shader_source_path_{
      "assets/shader_files/vertex_shader.vs"};
  const std::filesystem::path fragment_shader_source_path_{
      "assets/shader_files/fragment_shader.fs"};
};
}  // namespace screen_controller
#endif  // GRAPHICS_RENDERER_H
