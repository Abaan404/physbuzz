#version 460 core

layout(location = 0) in vec3 aPosition;

out vec3 texCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    texCoord = vec3(aPosition.xy, -aPosition.z);
    gl_Position = (u_Projection * mat4(mat3(u_View)) * vec4(aPosition, 1.0)).xyww;
}
