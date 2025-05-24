#version 460 core

#define MAGNITUDE 2.0f

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

layout(std140, binding = 1) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

in VS_OUT {
    vec3 normal;
} gs_in[];

void GenerateLine(int index) {
    vec4 worldPos = gl_in[index].gl_Position;
    vec4 normalOffset = vec4(gs_in[index].normal, 0.0) * MAGNITUDE;

    gl_Position = camera.projection * worldPos;
    EmitVertex();

    gl_Position = camera.projection * (worldPos + normalOffset);
    EmitVertex();

    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}
