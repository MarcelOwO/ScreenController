//
// Created by marce on 4/15/2025.
//

#include "shader.h"
#include <array>
#include <filesystem>
#include <unistd.h>
#include <linux/limits.h>

Shader::Shader() {
}

void Shader::init(const std::filesystem::path& vertex_path,
                  const std::filesystem::path& fragment_path) {
    std::cout << "creating shader" << "\n";

    std::string vertex_code{};
    std::string fragment_code{};

    std::ifstream v_shader_file{};
    std::ifstream f_shader_file{};

    std::array<char,PATH_MAX> result{};

    try {
        ssize_t count = readlink("/proc/self/exe", result.data(), PATH_MAX);

        auto executable_path = std::filesystem::path(
            std::string(result.data(), count > 0 ? count : 0));

        auto project_dir = executable_path.parent_path();

        v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);;

        std::filesystem::path v_path(project_dir / vertex_path);
        std::filesystem::path f_path(project_dir / fragment_path);

        if (!exists(v_path)) {
            std::cout << "Vertex shader file does not exist: " << v_path << std::endl;
            return;
        }
        if (!exists(f_path)) {
            std::cout << "Fragment shader file does not exist: " << f_path << std::endl;
            return;
        }

        v_shader_file.open(v_path);
        f_shader_file.open(f_path);

        std::stringstream v_shader_stream;
        std::stringstream f_shader_stream;

        v_shader_stream << v_shader_file.rdbuf();
        f_shader_stream << f_shader_file.rdbuf();

        v_shader_file.close();
        f_shader_file.close();

        vertex_code = v_shader_stream.str();
        fragment_code = f_shader_stream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }

    const char* v_shader_code = vertex_code.c_str();
    const char* f_shader_code = fragment_code.c_str();

    unsigned int vertex{};
    unsigned int fragment{};

    std::cout << "Creating shader program" << "\n";
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader_code, nullptr);
    glCompileShader(vertex);
    check_compile_errors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_shader_code, nullptr);
    glCompileShader(fragment);
    check_compile_errors(fragment, "FRAGMENT");

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    check_compile_errors(id, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    std::cout << "shader created" << "\n";
}

void Shader::use() const {
    glUseProgram(id);
}

void Shader::set_float(const std::string& name, const float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::set_bool(const std::string& name, const bool value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<int>(value));
}

void Shader::set_int(const std::string& name, const int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

GLint Shader::get_uniform_location(const std::string& name) const {
    return glGetUniformLocation(id, name.c_str());
}

void Shader::check_compile_errors(const unsigned int shader, const std::string& type) {
    int success{};
    std::array<char, 1024> info_log{};

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success == 0) {
            glGetShaderInfoLog(shader, 1024, nullptr, info_log.data());
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << info_log.
            data()
            << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (success == 0) {
            glGetProgramInfoLog(shader, 1024, nullptr, info_log.data());
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << info_log.data()
            <<
            "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
