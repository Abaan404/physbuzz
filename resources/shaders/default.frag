#version 460 core

#pbz_include "lighting/defines.glsl"
#pbz_include "lighting/phong.glsl"

uniform Material u_Material;
uniform Light u_Light;

uniform vec3 u_ViewPosition;

in vec3 normal;
in vec2 texCoord;
in vec3 fragPosition;

out vec4 fragColor;

void main() {
    vec3 result = calculatePhong(u_Material, u_Light, fragPosition, normal, texCoord, u_ViewPosition);

    fragColor = vec4(result, 1.0);
}
