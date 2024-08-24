#version 460 core

#pbz_include "lighting/point.glsl"
#pbz_include "lighting/directional.glsl"
#pbz_include "lighting/spot.glsl"

uniform Material u_Material;
uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_PointLight;
uniform SpotLight u_SpotLight;

uniform vec3 u_ViewPosition;

in vec3 normal;
in vec2 texCoord;
in vec3 fragPosition;

out vec4 fragColor;

void main() {
    vec3 result = calcDirectionalLight(u_Material, u_DirectionalLight, fragPosition, normal, texCoord, u_ViewPosition);
    // vec3 result = calcPointLight(u_Material, u_PointLight, fragPosition, normal, texCoord, u_ViewPosition);
    // vec3 result = calcSpotLight(u_Material, u_SpotLight, fragPosition, normal, texCoord, u_ViewPosition);

    fragColor = vec4(result, 1.0);
}
