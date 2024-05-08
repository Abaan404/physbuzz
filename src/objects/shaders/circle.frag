#include <string>

static const char *circleFrag = R"(
#version 460

in vec3 fragPosition;

uniform uint u_time;

out vec4 fragColor;

void main() {
    vec2 uv = (fragPosition.xy + 1.0) / 2.0;

    float scaled_time = u_time / 2000.0;
    fragColor = vec4(abs(sin(scaled_time + uv.x)), abs(sin(scaled_time + uv.y)), abs(sin(scaled_time - uv.x)),1.0);
}
)";
