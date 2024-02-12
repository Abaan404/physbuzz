#include <string>

static const std::string box_frag = R"(
#version 460

out vec4 fragColor;

uniform vec4 u_color = vec4(1.0, 1.0, 0.0, 0.0);

void main() {
    fragColor = vec4(u_color.xyz, 1.0);
}
)";
