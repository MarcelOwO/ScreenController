//
// Created by marce on 4/2/2025.
//

#include "graphics_renderer.h"

#include <array>
#include <bit>
#include <iostream>


GraphicsRenderer::GraphicsRenderer(): vertex_shader_(0), fragment_shader_(0), texture_(0),
                                      shader_program_(0), vao_(0), vbo_(0) {
    vertex_shader_source_ = R"(
#version 300 es
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

    fragment_shader_source_ = R"(
#version 300 es
precision mediump float;
in vec2 vTexCoord;
layout(location = 0) out vec4 fragColor;
uniform sampler2D uTexture;
void main() {
    fragColor = texture(uTexture, vTexCoord);
}
)";
}

GraphicsRenderer::~GraphicsRenderer() {
    cleanup();
}

void GraphicsRenderer::init(const GLADloadproc dloadproc, const int window_width,
                            const int window_height) {
    if (!gladLoadGLES2Loader(dloadproc)) {
        return;
    }

    glViewport(0, 0, window_width, window_height);

    vertex_shader_ = compile_shader(GL_VERTEX_SHADER, vertex_shader_source_);
    fragment_shader_ = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source_);
    shader_program_ = link_program(vertex_shader_, fragment_shader_);

    constexpr std::array vertices = {
        -1.0f, 1.0f, 0.0f, 1.0f, // Top-left
        -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
        1.0f, 1.0f, 1.0f, 1.0f, // Top-right
        1.0f, -1.0f, 1.0f, 0.0f // Bottom-right
    };


    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

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

    const GLubyte tex_data[] = {
        255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 255,
    };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GraphicsRenderer::render() const {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    const GLint tex_uniform_location = glGetUniformLocation(shader_program_, "uTexture");
    glUniform1i(tex_uniform_location, 0);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void GraphicsRenderer::cleanup() const {
    glDeleteTextures(1, &texture_);
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
    glDeleteProgram(shader_program_);
    glDeleteShader(vertex_shader_);
    glDeleteShader(fragment_shader_);
}

GLuint GraphicsRenderer::link_program(const GLuint vertex_shader, const GLuint fragment_shader) {
    const GLuint program = glCreateProgram();
    if (vertex_shader)
        glAttachShader(program, vertex_shader);
    if (fragment_shader)
        glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        const auto log = static_cast<char*>(malloc(log_length));
        glGetProgramInfoLog(program, log_length, nullptr, log);
        std::cout << "Program link error: " << log << '\n';
        free(log);

        glDeleteProgram(program);
        return 0;
    }
    return program;
}

GLuint GraphicsRenderer::compile_shader(const GLenum type, const std::string& source) {
    const char* source_copy = source.c_str();
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source_copy, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint log_length;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        const auto log = static_cast<char*>(malloc(log_length));

        glGetShaderInfoLog(shader, log_length, nullptr, log);

        std::cout << "Shader compile error: " << log << '\n';

        free(log);

        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
