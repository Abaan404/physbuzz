#version 460 core

layout (location = 0) in vec3 aPosition;

uniform mat4 PBZ_Model;

void main() {
    gl_Position = PBZ_Model * vec4(aPosition, 1.0);
}  
