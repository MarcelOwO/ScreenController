//
// Created by marce on 4/2/2025.
//
#pragma once

#include <glad/glad.h>
#include <graphics/renderer.hpp>
#include <logging/logger.hpp>
#include <models/frame_data.hpp>

#include <expected>

#include "shader/shader.hpp"

namespace screen_controller {

class GraphicsRenderer : public IRenderer {
public:
  static auto Create(ILogger& logger, ProcLoader dloadproc, int window_width, int window_height)
      -> std::expected<std::unique_ptr<GraphicsRenderer>, std::error_code>;

  void Stop();

  ~GraphicsRenderer() override;

  void SetTexture(const FrameData* data) override;
  void SetFallbackTexture() const override;
  void UpdateRatio(int width, int height) const override;

  void Rotate() const override;

  void Render() const override;

private:
  explicit GraphicsRenderer(ILogger& logger, Shader shader, GLuint texture, GLuint vao, GLuint vbo,
                            int width, int height);

  ILogger& logger_;

  Shader shader_;

  int height_;
  int width_;

  mutable float rotation_angle_{0.0F};

  GLuint texture_;
  GLuint vao_;
  GLuint vbo_;
};
}  // namespace screen_controller
