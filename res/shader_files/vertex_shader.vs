#version 300 es

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uRotation;

out vec2 vTexCoord;

void main() {
mat4 rotationMatrix = uRotation == mat4(0.0) ? mat4(1.0) : uRotation;
    gl_Position = rotationMatrix * vec4(aPosition, 0.0, 1.0);

    vTexCoord = aTexCoord;
}