static const char *cubeVertex = R"(
#version 460

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 normal;
out vec2 texCoord;


void main() {
    gl_Position = u_Projection * u_View * u_Model * vec4(aPosition, 1.0f);
    texCoord = aTexCoord;
}
)";
