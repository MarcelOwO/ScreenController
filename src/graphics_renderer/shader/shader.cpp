//
// Created by marce on 4/15/2025.
//

#include "shader.h"

#include <linux/limits.h>
#include <ng-log/logging.h>
#include <unistd.h>

#include <array>
#include <filesystem>

namespace screen_controller {
Shader::Shader() = default;

void Shader::init(const std::filesystem::path& vertex_path,
                  const std::filesystem::path& fragment_path) {
  LOG(INFO) << "Creating shaders";
  std::string vertex_code{};
  std::string fragment_code{};

  std::ifstream v_shader_file{};
  std::ifstream f_shader_file{};

  std::array<char, PATH_MAX> result{};

  ssize_t count = readlink("/proc/self/exe", result.data(), PATH_MAX);

  PCHECK(count >= 0) << "Failed to read link: " << strerror(errno);

  auto executable_path =
      std::filesystem::path(std::string(result.data(), count > 0 ? count : 0));

  auto project_dir = executable_path.parent_path();

  std::filesystem::path v_path(project_dir / vertex_path);
  std::filesystem::path f_path(project_dir / fragment_path);

  PCHECK(std::filesystem::exists(v_path))
      << "Vertex shader path does not exist: " << v_path;
  PCHECK(std::filesystem::exists(f_path))
      << "Fragment shader path does not exist: " << f_path;

  v_shader_file.open(v_path);
  f_shader_file.open(f_path);

  PCHECK(v_shader_file.is_open() && f_shader_file.is_open())
      << "Failed to open shader files: " << v_path << " " << f_path;

  std::stringstream v_shader_stream{};
  std::stringstream f_shader_stream{};

  v_shader_stream << v_shader_file.rdbuf();
  f_shader_stream << f_shader_file.rdbuf();

  v_shader_file.close();
  f_shader_file.close();

  vertex_code = v_shader_stream.str();
  fragment_code = f_shader_stream.str();

  GLenum err = glGetError();
  PCHECK(err == GL_NO_ERROR)
      << "OpenGL error before shader compilation: " << err;

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
      PLOG(ERROR) << "Shader compilation error: " << info_log.data();
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (success == 0) {
      glGetProgramInfoLog(shader, 1024, nullptr, info_log.data());
      PLOG(ERROR) << "Program linking error: " << info_log.data();
    }
  }
}
}  // namespace screen_controller
