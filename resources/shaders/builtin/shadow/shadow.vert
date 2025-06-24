#version 460 core

layout(location = 0) in vec3 aPosition;

uniform mat4 u_LightMatrix;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_LightMatrix * u_Model * vec4(aPosition, 1.0);
}
