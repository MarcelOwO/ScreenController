//
// Created by marce on 4/2/2025.
//

#include "graphics_renderer.hpp"

#include <array>
#include <bit>
#include <glm/mat4x4.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <models/frame_data.hpp>

#include <expected>

#include "glm/gtx/rotate_vector.hpp"
#include "shader/shader.hpp"

namespace screen_controller {

GraphicsRenderer::GraphicsRenderer(ILogger& logger, Shader shader, GLuint texture, GLuint vao,
                                   GLuint vbo, int width, int height)
    : logger_(logger),
      shader_(shader),
      texture_(texture),
      vao_(vao),
      vbo_(vbo),
      width_(width),
      height_(height) {
  logger_.LogInfo("Creating GraphicsRenderer");
}

auto GraphicsRenderer::Create(ILogger& logger, ProcLoader dloadproc, int window_width,
                              int window_height)
    -> std::expected<std::unique_ptr<GraphicsRenderer>, std::error_code> {
  if (gladLoadGLES2Loader(dloadproc) == 0) {
    logger.LogError("Renderer: Failed to load GLAD");
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  const std::filesystem::path kVertexShaderSourcePath{"res/shader_files/vertex_shader.vs"};

  const std::filesystem::path kFragmentShaderSourcePath{"res/shader_files/fragment_shader.fs"};

  auto shader = Shader::Create(logger, kVertexShaderSourcePath, kFragmentShaderSourcePath);

  if (!shader) {
    logger.LogError("Failed to init shader");
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  glViewport(0, 0, window_width, window_height);

  constexpr std::array kVertices = {
      -1.0F, 1.0F,  0.0F, 0.0F,  // Top-left
      -1.0F, -1.0F, 0.0F, 1.0F,  // Bottom-left
      1.0F,  1.0F,  1.0F, 0.0F,  // Top-right
      1.0F,  -1.0F, 1.0F, 1.0F   // Bottom-right
  };

  GLuint texture;
  GLuint vao;
  GLuint vbo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), static_cast<void*>(nullptr));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                        std::bit_cast<void*>(2 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  auto graphics_renderer = std::unique_ptr<GraphicsRenderer>(
      new GraphicsRenderer(logger, shader.value(), texture, vao, vbo, window_width, window_height));

  if (!graphics_renderer) {
    logger.LogError("Renderer: Graphics Renderer returned null");
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  return graphics_renderer;
}

GraphicsRenderer::~GraphicsRenderer() {
  logger_.LogInfo("Cleaning up GraphicsRenderer");
  glDeleteTextures(1, &texture_);
  glDeleteBuffers(1, &vbo_);
  glDeleteVertexArrays(1, &vao_);
}

void GraphicsRenderer::Render() const {
  glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_);

  shader_.Run();
  glUniform1i(glGetUniformLocation(shader_.program_id_, "uTexture"), 0);

  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void GraphicsRenderer::SetTexture(const FrameData* data) {
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE,
               data->data.data());
  glGenerateMipmap(GL_TEXTURE_2D);
}
void GraphicsRenderer::SetFallbackTexture() const {
  logger_.LogInfo("Setting fallback texture");
  const std::vector<unsigned char> kBlackImage(width_ * height_ * 3,
                                               0);  // RGB black image

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE,
               kBlackImage.data());
  glGenerateMipmap(GL_TEXTURE_2D);
}
void GraphicsRenderer::UpdateRatio(const int kWidth, const int kHeight) const {
  const float kImageAspect = static_cast<float>(kWidth) / kHeight;
  const float kScreenAspect = static_cast<float>(1920) / 1080;

  float plane_width = 1.0F;
  float plane_height = 1.0F;

  // Adjust plane dimensions to fit the screen
  if (kImageAspect > kScreenAspect) {
    plane_width = 1.0F;
    plane_height = kScreenAspect / kImageAspect;
  } else {
    plane_height = 1.0F;
    plane_width = kImageAspect / kScreenAspect;
  }

  const std::array<float, 16> kVertices = {
      -plane_width, plane_height,  0.0F, 0.0F,  // Top-left
      -plane_width, -plane_height, 0.0F, 1.0F,  // Bottom-left
      plane_width,  plane_height,  1.0F, 0.0F,  // Top-right
      plane_width,  -plane_height, 1.0F, 1.0F   // Bottom-right
  };

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GraphicsRenderer::Rotate() const {
  constexpr float kStepDegrees = 90.0F;
  rotation_angle_ += glm::radians(kStepDegrees);

  const glm::mat4 kRotation =
      glm::rotate(glm::mat4(1.0F), rotation_angle_, glm::vec3(0.0F, 0.0F, 1.0F));

  shader_.SetMat4("uRotation", kRotation);
}
}  // namespace screen_controller
