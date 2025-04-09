//
// Created by marce on 4/2/2025.
//

#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include <string>
#include <iostream>

#include <glad.h>

class GraphicsRenderer
{
public:
    GraphicsRenderer();
    ~GraphicsRenderer();

    void init(GLADloadproc dloadproc, int windowWidth, int windowHeight);

    void render();
    void cleanup();

private:
    std::string vertexShaderSource;
    std::string fragmentShaderSource;

    GLuint vertexShader;
    GLuint fragmentShader;

    GLuint texture;

    GLuint shaderProgram;

    GLuint vao;
    GLuint vbo;

    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
    GLuint compileShader(GLenum type, std::string source);
};


#endif //GRAPHICS_RENDERER_H
