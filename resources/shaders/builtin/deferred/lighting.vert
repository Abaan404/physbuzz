#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out VS_OUT {
    vec2 texCoord;
} vs_out;

void main() {
    vs_out.texCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}
