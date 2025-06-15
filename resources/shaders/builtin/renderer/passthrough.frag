#version 460 core

uniform sampler2D u_ScreenTexture;

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

void main() {
    fragColor = vec4(texture(u_ScreenTexture, fs_in.texCoord).rgb, 1.0f);
}
