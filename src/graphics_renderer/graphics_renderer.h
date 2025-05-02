//
// Created by marce on 4/2/2025.
//

#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include <glad/glad.h>

#include <span>
#include <string>

#include "../common/pixel_data.h"
#include "shader/shader.h"

namespace screen_controller {
class GraphicsRenderer {
 public:
  GraphicsRenderer();
  ~GraphicsRenderer();

  struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  void init(GLADloadproc dloadproc, int window_width, int window_height);

  void set_texture(std::span<const std::byte> data);
  void render() const;


 private:
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
