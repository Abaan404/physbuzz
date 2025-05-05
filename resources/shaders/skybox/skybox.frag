#version 460 core

uniform samplerCube u_Skybox;

in vec3 texCoord;

out vec4 fragColor;

void main()
{
    fragColor = texture(u_Skybox, texCoord);
}
