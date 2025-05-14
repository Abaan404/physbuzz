#version 460 core

layout(std140, binding = 2) uniform Window {
    ivec2 resolution;
} window;

layout(std140, binding = 3) uniform Time {
    uint time;
    uint timedelta;
} time;

uniform sampler2D u_Texture;

in VS_OUT {
    vec3 normal;
    vec2 texCoord;
    vec3 fragPosition;
} fs_in;

out vec4 fragColor;

void main() {
    vec2 uv = ((fs_in.fragPosition.xy / window.resolution) + 1.0) / 2.0;

    float scaled_time = time.time / 2000.0f;
    fragColor = vec4(abs(sin(scaled_time + uv.x)), abs(sin(scaled_time + uv.y)), abs(sin(scaled_time - uv.x)), 1.0);
    fragColor = mix(texture(u_Texture, fs_in.texCoord), fragColor, 0.6);
}
