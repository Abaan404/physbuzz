#version 460 core

uniform sampler2D PBZ_Framebuffer;

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

void main() {
    float depthValue = texture(PBZ_Framebuffer, fs_in.texCoord).r;
    fragColor = vec4(vec3(1.0 - depthValue), 1.0);
    // fragColor = vec4(texture(PBZ_Framebuffer, fs_in.texCoord).rgb, 1.0f);
}
