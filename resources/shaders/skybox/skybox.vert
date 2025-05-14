#version 460 core

layout(location = 0) in vec3 aPosition;

layout(std140, binding = 1) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

out VS_OUT {
    vec3 texCoord;
} vs_out;

void main()
{
    vs_out.texCoord = vec3(aPosition.xy, -aPosition.z);
    gl_Position = (camera.projection * mat4(mat3(camera.view)) * vec4(aPosition, 1.0)).xyww;
}
