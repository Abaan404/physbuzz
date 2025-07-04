#version 460 core

#pbz_include "../common/lighting/directional.glsl"

#define MAX_DIRECTIONAL_LIGHTS 100

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec2 aTexCoord;

layout(std140, binding = 1) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform mat4 u_Model;

uniform uint u_DirectionalLightLength;
uniform DirectionalLight u_DirectionalLight[MAX_DIRECTIONAL_LIGHTS];

out VS_OUT {
    vec4 fragPositionLightSpace;
    vec3 normal;
    vec3 fragPosition;
    vec2 texCoord;
} vs_out;

void main() {
    gl_Position = camera.projection * camera.view * u_Model * vec4(aPosition, 1.0f);

    vs_out.texCoord = aTexCoord;
    vs_out.normal = mat3(transpose(inverse(u_Model))) * aNormal;
    vs_out.fragPosition = vec3(u_Model * vec4(aPosition, 1.0));
    vs_out.fragPositionLightSpace = u_DirectionalLight[0].matrix * vec4(vs_out.fragPosition, 1.0);
}
