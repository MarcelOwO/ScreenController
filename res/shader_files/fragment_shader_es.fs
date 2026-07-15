#version 300 es

precision mediump float;

in vec2 vTexCoord;
layout(location = 0) out vec4 fragColor;

uniform sampler2D uTexture;
uniform float uBrightness;

void main() {
    vec4 color = texture(uTexture, vTexCoord);
    fragColor = vec4(color.rgb * uBrightness, color.a);
}
