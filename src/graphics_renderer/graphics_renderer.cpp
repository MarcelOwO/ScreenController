//
// Created by marce on 4/2/2025.
//

#include "graphics_renderer.h"

#include <algorithm>
#include <array>
#include <bit>
#include <iostream>
#include <span>

#include "shader/shader.h"

namespace screen_controller {
GraphicsRenderer::GraphicsRenderer() : texture_(), vao_(), vbo_() {}

GraphicsRenderer::~GraphicsRenderer() {
  glDeleteTextures(1, &texture_);
  glDeleteBuffers(1, &vbo_);
  glDeleteVertexArrays(1, &vao_);
}

void GraphicsRenderer::init(const GLADloadproc dloadproc,
                            const int window_width, const int window_height) {
  if (!gladLoadGLES2Loader(dloadproc)) {
    return;
  }

  shader_.init(vertex_shader_source_path_, fragment_shader_source_path_);

  glViewport(0, 0, window_width, window_height);

  constexpr std::array vertices = {
      -1.0F, 1.0F,  0.0F, 0.0F,  // Top-left
      -1.0F, -1.0F, 0.0F, 1.0F,  // Bottom-left
      1.0F,  1.0F,  1.0F, 0.0F,  // Top-right
      1.0F,  -1.0F, 1.0F, 1.0F   // Bottom-right
  };

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                        static_cast<void*>(nullptr));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                        std::bit_cast<void*>(2 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void GraphicsRenderer::render() const {
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_);

  shader_.use();
  glUniform1i(glGetUniformLocation(shader_.id_, "uTexture"), 0);

  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void GraphicsRenderer::set_texture(const std::span<const uint8_t> data) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB,
               GL_UNSIGNED_BYTE, data.data());

  glGenerateMipmap(GL_TEXTURE_2D);
}
}  // namespace screen_controller
