#version 460 core

uniform sampler2D PBZ_Framebuffer;

in VS_OUT {
    vec2 texCoord;
} fs_in;

layout(location = 0) out vec4 fragColor;

void main() {
    float gamma = 2.2f;
    float exposure = 1.0f;

    vec3 color = texture(PBZ_Framebuffer, fs_in.texCoord).rgb;
    vec3 mapped = vec3(1.0) - exp(-color * exposure);

    fragColor.rgb = pow(mapped, vec3(1.0f / gamma));
}
