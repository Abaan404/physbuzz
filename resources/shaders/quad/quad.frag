#version 460 core

uniform vec4 u_Color = vec4(1.0, 1.0, 0.0, 0.0);
uniform sampler2D u_Texture;

in VS_OUT {
    vec3 normal;
    vec3 fragPosition;
    vec2 texCoord;
} fs_in;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(u_Texture, fs_in.texCoord);
}
