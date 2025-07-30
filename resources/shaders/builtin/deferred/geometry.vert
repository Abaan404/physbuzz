#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoord;

layout(std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform mat4 PBZ_Model;

out VS_OUT {
    mat3 TBN;
    vec3 normal;
    vec3 fragPosition;
    vec2 texCoord;
} vs_out;

void main() {
    gl_Position = camera.projection * camera.view * PBZ_Model * vec4(aPosition, 1.0f);

    vec3 T = vec3(PBZ_Model * vec4(aTangent, 0.0));
    vec3 N = vec3(PBZ_Model * vec4(aNormal, 0.0));
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
    vs_out.normal = mat3(transpose(inverse(PBZ_Model))) * aNormal;
    vs_out.fragPosition = vec3(PBZ_Model * vec4(aPosition, 1.0));
    vs_out.texCoord = aTexCoord;
}
