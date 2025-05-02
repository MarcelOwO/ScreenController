//
// Created by marce on 4/15/2025.
//

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace screen_controller {
class Shader {
 public:
  unsigned int id_;

  Shader();

  void init(const std::filesystem::path& vertex_path,
            const std::filesystem::path& fragment_path);

  void use() const;
  void set_float(const std::string& name, float value) const;
  void set_bool(const std::string& name, bool value) const;
  void set_int(const std::string& name, int value) const;
  [[nodiscard]] GLint get_uniform_location(const std::string& name) const;

 private:
  static void check_compile_errors(unsigned int shader,
                                   const std::string& type);
};
}  // namespace screen_controller
#endif  // SHADER_H
