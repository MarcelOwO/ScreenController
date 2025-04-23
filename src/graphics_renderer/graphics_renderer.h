//
// Created by marce on 4/2/2025.
//

#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include <string>
#include <glad/glad.h>

#include "shader/shader.h"

#include "../common/pixel_data.h"

class GraphicsRenderer {
public:
    GraphicsRenderer();
    ~GraphicsRenderer();

    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };


    void init(GLADloadproc dloadproc, int window_width, int window_height);
    void set_texture(const std::array<PixelData, 2073600>& data);
    void render() const;


    void cleanup();

private:
    Shader shader_;

    GLuint texture_;

    GLuint vao_;
    GLuint vbo_;

    const std::filesystem::path vertex_shader_source_path_{"assets/shader_files/vertex_shader.vs"};
    const std::filesystem::path fragment_shader_source_path_{
        "assets/shader_files/fragment_shader.fs"
    };
};


#endif //GRAPHICS_RENDERER_H
