//
// Created by marce on 4/2/2025.
//

#include "include/graphics_renderer.h"


#include <array>
#include <bit>
#include <glm/mat4x4.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <expected>

#include "../common/models/frame_data.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "models/error_enum.h"
#include "shader/shader.h"

namespace screen_controller {

GraphicsRenderer::GraphicsRenderer(const std::shared_ptr<Logger>& logger) : logger_(logger), texture_(), vao_(), vbo_() {
  logger_->LogInfo("Creating GraphicsRenderer");
}

GraphicsRenderer::~GraphicsRenderer() {
  logger_->LogInfo("Cleaning up GraphicsRenderer");
  glDeleteTextures(1, &texture_);
  glDeleteBuffers(1, &vbo_);
  glDeleteVertexArrays(1, &vao_);
}

std::expected<void, common::ErrorEnum> GraphicsRenderer::init(
    const GLADloadproc dloadproc, const int window_width,
    const int window_height) {
  if (gladLoadGLES2Loader(dloadproc)==0) {
   logger_->LogError("Failed to load GLAD");
    return std::unexpected(common::ErrorEnum::ERROR);
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

void GraphicsRenderer::set_texture(const common::FrameData* data) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB,
               GL_UNSIGNED_BYTE, data->data.data());
  glGenerateMipmap(GL_TEXTURE_2D);
}
void GraphicsRenderer::set_fallback_texture() {
  logger_->LogInfo("Settig fallback texture");
  constexpr int width = 1920;
  constexpr int height = 1080;
  std::vector<unsigned char> black_image(width * height * 3,
                                         0);  // RGB black image

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, black_image.data());
  glGenerateMipmap(GL_TEXTURE_2D);
}
void GraphicsRenderer::update_ratio(int width, int height) {
  const float image_aspect = static_cast<float>(width) / height;
  const float screen_aspect = static_cast<float>(1920) / 1080;

  float plane_width = 1.0F;
  float plane_height = 1.0F;

  // Adjust plane dimensions to fit the screen
  if (image_aspect > screen_aspect) {
    plane_width = 1.0F;
    plane_height = screen_aspect / image_aspect;
  } else {
    plane_height = 1.0F;
    plane_width = image_aspect / screen_aspect;
  }

  const std::array<float, 16> vertices = {
      -plane_width, plane_height,  0.0F, 0.0F,  // Top-left
      -plane_width, -plane_height, 0.0F, 1.0F,  // Bottom-left
      plane_width,  plane_height,  1.0F, 0.0F,  // Top-right
      plane_width,  -plane_height, 1.0F, 1.0F   // Bottom-right
  };

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GraphicsRenderer::rotate() {
  static float angle = 90.0F;

  angle += glm::radians(90.0F);  // Increment rotation by 90 degrees

  const glm::mat4 rotation =
      glm::rotate(glm::mat4(1.0F), angle, glm::vec3(0.0F, 0.0F, 1.0F));

  shader_.set_mat4("uRotation", rotation);
}
}  // namespace screen_controller
