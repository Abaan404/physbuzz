#version 460 core

uniform samplerCube u_Skybox;

in VS_OUT {
    vec3 texCoord;
} fs_in;

out vec4 fragColor;

void main()
{
    fragColor = texture(u_Skybox, fs_in.texCoord);
}
