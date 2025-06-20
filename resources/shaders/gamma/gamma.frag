#version 460 core

uniform sampler2D PBZ_Framebuffer;

in VS_OUT {
    vec2 texCoord;
} fs_in;

layout(location = 0) out vec4 fragColor;

void main() {
    vec4 color = texture(PBZ_Framebuffer, fs_in.texCoord);

    float gamma = 2.2f;
    fragColor.rgb = pow(color.rgb, vec3(1.0f / gamma));
}
