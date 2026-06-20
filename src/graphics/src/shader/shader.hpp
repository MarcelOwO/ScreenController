//
// Created by marce on 4/15/2025.
//
#pragma once

#include <glad/glad.h>
#include <logging/logger.hpp>

#include <expected>
#include <filesystem>
#include <string>

#include "glm/fwd.hpp"

namespace screen_controller {
class Shader {
public:
  static std::expected<Shader, std::error_code> Create(ILogger& logger,
                                                       const std::filesystem::path& vertex_path,
                                                       const std::filesystem::path& fragment_path);

  void Run() const;

  unsigned int program_id_;

  void SetFloat(const std::string& name, float value) const;
  void SetBool(const std::string& name, bool value) const;
  void SetInt(const std::string& name, int value) const;
  void SetMat4(const std::string& name, glm::mat4 value) const;

  [[nodiscard]] GLint GetUniformLocation(const std::string& name) const;

private:
  explicit Shader(ILogger& logger, int program_id);

  ILogger& logger_;

  static void CheckCompileErrors(unsigned int shader, const std::string& type, ILogger& logger);
};
}  // namespace screen_controller
