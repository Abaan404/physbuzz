#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 3) in vec2 aTexCoord;

layout(std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform mat4 u_Model;

out VS_OUT {
    vec3 fragPosition;
    vec2 texCoord;
} vs_out;

void main() {
    gl_Position = camera.projection * camera.view * u_Model * vec4(aPosition, 1.0f);

    vs_out.texCoord = aTexCoord;
    vs_out.fragPosition = vec3(u_Model * vec4(aPosition, 1.0));
}
