//
// Created by marce on 4/15/2025.
//

#include "shader.h"

#include <linux/limits.h>
#include <unistd.h>

#include <array>
#include <filesystem>
#include <fstream>
#define GLM_ENABLE_EXPERIMENTAL

#include <expected>

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"

namespace screen_controller {

Shader::Shader(ILogger& logger, int program_id) : logger_(logger), program_id_(program_id) {
  logger_.LogInfo("Creating shader");
}

std::expected<Shader, std::error_code> Shader::Create(ILogger& logger,
                                                      const std::filesystem::path& vertex_path,
                                                      const std::filesystem::path& fragment_path) {
  logger.LogInfo("Initializing shaders");

  std::array<char, PATH_MAX> result{};

  ssize_t count = readlink("/proc/self/exe", result.data(), PATH_MAX);

  if (count < 0) {
    logger.LogError("Failed to read link " + std::string(strerror(errno)));
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  auto executable_path = std::filesystem::path(std::string(result.data(), count > 0 ? count : 0));

  auto project_dir = executable_path.parent_path();

  std::filesystem::path v_path(project_dir / vertex_path);
  std::filesystem::path f_path(project_dir / fragment_path);

  if (!std::filesystem::exists(v_path)) {
    logger.LogError("Vertex shader path does not exist: " + f_path.string());
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  if (!std::filesystem::exists(f_path)) {
    logger.LogError("Fragment shader path does not exist: " + f_path.string());
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  std::ifstream v_shader_file{};
  std::ifstream f_shader_file{};

  v_shader_file.open(v_path);
  f_shader_file.open(f_path);

  if (!v_shader_file.is_open() || !f_shader_file.is_open()) {
    logger.LogError("Failed to open shader files" + v_path.string() + " " + f_path.string());
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  std::stringstream v_shader_stream{};
  std::stringstream f_shader_stream{};

  v_shader_stream << v_shader_file.rdbuf();
  f_shader_stream << f_shader_file.rdbuf();

  v_shader_file.close();
  f_shader_file.close();

  std::string vertex_code = v_shader_stream.str();
  std::string fragment_code = f_shader_stream.str();

  if (GLenum err = glGetError(); err != GL_NO_ERROR) {
    logger.LogError("OpenGL error before shader compilation: " + std::to_string(err));
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }

  const char* v_shader_code = vertex_code.c_str();
  const char* f_shader_code = fragment_code.c_str();

  unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &v_shader_code, nullptr);
  glCompileShader(vertex);
  Shader::CheckCompileErrors(vertex, "VERTEX", logger);

  unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &f_shader_code, nullptr);
  glCompileShader(fragment);
  Shader::CheckCompileErrors(fragment, "FRAGMENT", logger);

  auto program_id = glCreateProgram();
  glAttachShader(program_id, vertex);
  glAttachShader(program_id, fragment);
  glLinkProgram(program_id);

  Shader::CheckCompileErrors(program_id, "PROGRAM", logger);

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  return Shader(logger, program_id);
}

void Shader::Run() const {
  glUseProgram(program_id_);
}

void Shader::SetFloat(const std::string& name, const float kValue) const {
  glUniform1f(glGetUniformLocation(program_id_, name.c_str()), kValue);
}

void Shader::SetBool(const std::string& name, const bool kValue) const {
  glUniform1i(glGetUniformLocation(program_id_, name.c_str()), static_cast<int>(kValue));
}

void Shader::SetInt(const std::string& name, const int kValue) const {
  glUniform1i(glGetUniformLocation(program_id_, name.c_str()), kValue);
}
void Shader::SetMat4(const std::string& name, glm::mat4 value) const {
  glUniformMatrix4fv(glGetUniformLocation(program_id_, name.c_str()), 1, GL_FALSE,
                     glm::value_ptr(value));
}

GLint Shader::GetUniformLocation(const std::string& name) const {
  return glGetUniformLocation(program_id_, name.c_str());
}

void Shader::CheckCompileErrors(const unsigned int kShader, const std::string& type,
                                ILogger& logger) {
  int success{};

  constexpr size_t kMaxLogSize = 1024;
  std::array<char, kMaxLogSize> info_log{};

  if (type != "PROGRAM") {
    glGetShaderiv(kShader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
      glGetShaderInfoLog(kShader, kMaxLogSize, nullptr, info_log.data());
      logger.LogError("Shader compilation error: " + std::string(info_log.data()));
    }
  } else {
    glGetProgramiv(kShader, GL_LINK_STATUS, &success);
    if (success == 0) {
      glGetProgramInfoLog(kShader, kMaxLogSize, nullptr, info_log.data());
      logger.LogError("Program linking error: " + std::string(info_log.data()));
    }
  }
}
}  // namespace screen_controller
