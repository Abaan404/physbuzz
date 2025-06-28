#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 PBZ_LightMatrix[6];

out GS_OUT {
    vec4 fragPosition;
} gs_out;

void main() {
    for (int face = 0; face < 6; face++) {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i) {
            gs_out.fragPosition = gl_in[i].gl_Position;
            gl_Position = PBZ_LightMatrix[face] * gs_out.fragPosition;
            EmitVertex();
        }
        EndPrimitive();
    }
}
