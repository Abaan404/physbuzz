#include <string>

static const std::string box_vertex = R"(
#version 460

layout (location = 0) in vec3 position;

void main() {
    gl_Position = vec4(position.xyz, 1.0);
}
)";
