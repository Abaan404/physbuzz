#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec2 aTexCoord;

out VS_OUT {
    vec3 normal;
} vs_out;

layout(std140, binding = 1) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform mat4 u_Model;

void main() {
    gl_Position = camera.view * u_Model * vec4(aPosition, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(camera.view * u_Model)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
}
