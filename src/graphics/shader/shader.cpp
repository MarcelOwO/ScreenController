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
#include "models/error_enum.h"

namespace screen_controller {
Shader::Shader(ILogger& logger) : logger_(logger) {
  logger_.LogInfo("Creating shader");
}

std::expected<void, common::ErrorEnum> Shader::init(
    const std::filesystem::path& vertex_path,
    const std::filesystem::path& fragment_path) {
  logger_.LogInfo("Initializing shaders");

  std::string vertex_code{};
  std::string fragment_code{};

  std::ifstream v_shader_file{};
  std::ifstream f_shader_file{};

  std::array<char, PATH_MAX> result{};

  ssize_t count = readlink("/proc/self/exe", result.data(), PATH_MAX);
  if (count < 0) {
    logger_.LogError("Failed to read link " + std::string(strerror(errno)));
    return std::unexpected(common::ErrorEnum::ERROR);
  }

  auto executable_path =
      std::filesystem::path(std::string(result.data(), count > 0 ? count : 0));

  auto project_dir = executable_path.parent_path();

  std::filesystem::path v_path(project_dir / vertex_path);
  std::filesystem::path f_path(project_dir / fragment_path);

  if (!std::filesystem::exists(v_path)) {
    logger_.LogError("Vertex shader path does not exist: " + f_path.string());
    return std::unexpected(common::ErrorEnum::ERROR);
  }

  if (!std::filesystem::exists(f_path)) {
    logger_.LogError("Fragment shader path does not exist: " + f_path.string());
    return std::unexpected(common::ErrorEnum::ERROR);
  }

  v_shader_file.open(v_path);
  f_shader_file.open(f_path);
  if (!v_shader_file.is_open() || !f_shader_file.is_open()) {
    logger_.LogError("Failed to open shader files" + v_path.string() + " " +
                     f_path.string());
    return std::unexpected(common::ErrorEnum::ERROR);
  }

  std::stringstream v_shader_stream{};
  std::stringstream f_shader_stream{};

  v_shader_stream << v_shader_file.rdbuf();
  f_shader_stream << f_shader_file.rdbuf();

  v_shader_file.close();
  f_shader_file.close();

  vertex_code = v_shader_stream.str();
  fragment_code = f_shader_stream.str();

  if (GLenum err = glGetError(); err != GL_NO_ERROR) {
    logger_.LogError("OpenGL error before shader compilation: " +
                     std::to_string(err));
    return std::unexpected(common::ErrorEnum::ERROR);
  }

  const char* v_shader_code = vertex_code.c_str();
  const char* f_shader_code = fragment_code.c_str();

  unsigned int vertex{};
  unsigned int fragment{};

  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &v_shader_code, nullptr);
  glCompileShader(vertex);
  check_compile_errors(vertex, "VERTEX");

  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &f_shader_code, nullptr);
  glCompileShader(fragment);
  check_compile_errors(fragment, "FRAGMENT");

  id_ = glCreateProgram();
  glAttachShader(id_, vertex);
  glAttachShader(id_, fragment);
  glLinkProgram(id_);

  check_compile_errors(id_, "PROGRAM");
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  return {};
}

void Shader::use() const { glUseProgram(id_); }

void Shader::set_float(const std::string& name, const float value) const {
  glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}

void Shader::set_bool(const std::string& name, const bool value) const {
  glUniform1i(glGetUniformLocation(id_, name.c_str()), static_cast<int>(value));
}

void Shader::set_int(const std::string& name, const int value) const {
  glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}
void Shader::set_mat4(const std::string& name, glm::mat4 value) const {
  glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE,
                     glm::value_ptr(value));
}

GLint Shader::get_uniform_location(const std::string& name) const {
  return glGetUniformLocation(id_, name.c_str());
}

void Shader::check_compile_errors(const unsigned int shader,
                                  const std::string& type) {
  int success{};
  std::array<char, 1024> info_log{};

  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0) {
      glGetShaderInfoLog(shader, 1024, nullptr, info_log.data());
      logger_.LogError("Shader compilation error: " +
                       std::string(info_log.data()));
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (success == 0) {
      glGetProgramInfoLog(shader, 1024, nullptr, info_log.data());
      logger_.LogError("Program linking error: " +
                       std::string(info_log.data()));
    }
  }
}
}  // namespace screen_controller
