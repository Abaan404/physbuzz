#version 460 core

uniform vec4 u_Color = vec4(1.0, 1.0, 0.0, 0.0);
uniform sampler2D u_Texture;

in vec3 normal;
in vec2 texCoord;

out vec4 fragColor;

void main() {
    fragColor = texture(u_Texture, texCoord);
}
