#version 460 core

uniform ivec2 u_Resolution;
uniform sampler2D u_Texture;
uniform uint u_Time;

in vec3 fragPosition;
in vec3 normal;
in vec2 texCoord;

out vec4 fragColor;

void main() {
    vec2 uv = ((fragPosition.xy / u_Resolution) + 1.0) / 2.0;

    float scaled_time = u_Time / 2000.0;
    fragColor = vec4(abs(sin(scaled_time + uv.x)), abs(sin(scaled_time + uv.y)), abs(sin(scaled_time - uv.x)), 1.0);
    fragColor = mix(texture(u_Texture, texCoord), fragColor, 0.6);
}
