#version 460 core

uniform sampler2D u_Texture;

in vec2 texCoord;

out vec4 fragColor;

void main() {
    fragColor = texture(u_Texture, texCoord);
}
