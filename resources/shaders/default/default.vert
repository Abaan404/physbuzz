#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(std140, binding = 1) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform mat4 u_Model;

out VS_OUT {
    vec3 normal;
    vec2 texCoord;
    vec3 fragPosition;
} vs_out;

void main() {
    gl_Position = camera.projection * camera.view * u_Model * vec4(aPosition, 1.0f);

    vs_out.texCoord = aTexCoord;
    vs_out.normal = mat3(transpose(inverse(u_Model))) * aNormal;
    vs_out.fragPosition = vec3(u_Model * vec4(aPosition, 1.0));
}
