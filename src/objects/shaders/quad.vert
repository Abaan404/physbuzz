static const char *quadVertex = R"(
#version 460

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 normal;
out vec2 texCoord;

void main() {
    gl_Position = vec4(aPosition.xyz, 1.0);
    texCoord = aTexCoord;
}
)";
