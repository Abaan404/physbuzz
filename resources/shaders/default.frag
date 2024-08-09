#version 460

uniform sampler2D u_Texture;

in vec2 texCoord;

out vec4 fragColor;

void main() {
    fragColor = texture(u_Texture, texCoord);
}
