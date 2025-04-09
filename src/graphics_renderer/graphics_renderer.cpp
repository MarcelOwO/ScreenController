//
// Created by marce on 4/2/2025.
//

#include "graphics_renderer.h"

#include <array>
#include <bit>


GraphicsRenderer::GraphicsRenderer(): vertexShader(0), fragmentShader(0), texture(0), shaderProgram(0), vao(0), vbo(0)
{
    vertexShaderSource = R"(
#version 300 es
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 vTexCoord;
void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

    fragmentShaderSource = R"(
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

GraphicsRenderer::~GraphicsRenderer()
{
    cleanup();
}

void GraphicsRenderer::init(GLADloadproc dloadproc, int windowWidth, int windowHeight)
{
    if (!gladLoadGLES2Loader(dloadproc))
    {
        return;
    }

    glViewport(0, 0, windowWidth, windowHeight);

    vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    shaderProgram = linkProgram(vertexShader, fragmentShader);

    constexpr std::array<GLfloat,16> vertices = {
        -1.0f, 1.0f, 0.0f, 1.0f, // Top-left
        -1.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
        1.0f, 1.0f, 1.0f, 1.0f, // Top-right
        1.0f, -1.0f, 1.0f, 0.0f // Bottom-right
    };


    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), static_cast<void*>(nullptr));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), std::bit_cast<void*>(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const GLubyte texData[] = {
        255, 0, 0, 255,255, 0, 0, 255,255, 0, 0, 255,255, 0, 0, 255,
    };

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GraphicsRenderer::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    const GLint texUniformLocation = glGetUniformLocation(shaderProgram, "uTexture");
    glUniform1i(texUniformLocation, 0);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void GraphicsRenderer::cleanup()
{

    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

GLuint GraphicsRenderer::linkProgram(const GLuint vertexShader, const GLuint fragmentShader)
{
    const GLuint program = glCreateProgram();
    if (vertexShader)
        glAttachShader(program, vertexShader);
    if (fragmentShader)
        glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        char* log = (char*)malloc(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log);
        std::cout << "Program link error: " << log << '\n';
        free(log);

        glDeleteProgram(program);
        return 0;
    }
    return program;
}

GLuint GraphicsRenderer::compileShader(GLenum type, std::string source)
{
    const char * sourceCopy = source.c_str();
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &sourceCopy, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint logLength;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        char* log = (char*)malloc(logLength);

        glGetShaderInfoLog(shader, logLength, nullptr, log);

        std::cout << "Shader compile error: " << log << '\n';

        free(log);

        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
