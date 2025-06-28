#version 460 core

uniform vec3 PBZ_LightPosition;
uniform float PBZ_FarPlane;

in GS_OUT {
    vec4 fragPosition;
} fs_in;

void main() {
    float lightDistance = length(fs_in.fragPosition.xyz - PBZ_LightPosition);
    lightDistance = lightDistance / PBZ_FarPlane;
    gl_FragDepth = lightDistance;
}
