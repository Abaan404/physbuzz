#include <string>

static const char *circleVertex = R"(
#version 460

layout (location = 0) in vec3 position;

out vec3 fragPosition;

void main() {
    gl_Position = vec4(position.xyz, 1.0);
    fragPosition = position;
}
)";

