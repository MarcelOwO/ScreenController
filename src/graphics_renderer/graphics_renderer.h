//
// Created by marce on 4/2/2025.
//

#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include <string>

#include <glad/glad.h>

class GraphicsRenderer {
public:
    GraphicsRenderer();
    ~GraphicsRenderer();

    void init(GLADloadproc dloadproc, int window_width, int window_height);

    void render() const;
    void cleanup() const;

private:
    std::string vertex_shader_source_;
    std::string fragment_shader_source_;

    GLuint vertex_shader_;
    GLuint fragment_shader_;

    GLuint texture_;

    GLuint shader_program_;

    GLuint vao_;
    GLuint vbo_;

    static GLuint link_program(GLuint vertex_shader, GLuint fragment_shader);
    static GLuint compile_shader(GLenum type, const std::string& source);
};


#endif //GRAPHICS_RENDERER_H
