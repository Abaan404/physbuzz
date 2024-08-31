#version 460 core

#define MAX_POINT_LIGHTS 100

#pbz_include "lighting/point.glsl"
#pbz_include "lighting/directional.glsl"
#pbz_include "lighting/spot.glsl"

uniform uint u_PointLightLength;
uniform PointLight u_PointLight[MAX_POINT_LIGHTS];

uniform Material u_Material;
uniform DirectionalLight u_DirectionalLight;
uniform SpotLight u_SpotLight;

uniform vec3 u_ViewPosition;

in vec3 normal;
in vec2 texCoord;
in vec3 fragPosition;

out vec4 fragColor;

void main() {
    vec3 result;

    result += calcDirectionalLight(u_Material, u_DirectionalLight, fragPosition, normal, texCoord, u_ViewPosition);
    result += calcSpotLight(u_Material, u_SpotLight, fragPosition, normal, texCoord, u_ViewPosition);

    for (int i = 0; i < u_PointLightLength; i++) {
        result += calcPointLight(u_Material, u_PointLight[i], fragPosition, normal, texCoord, u_ViewPosition);
    }

    fragColor = vec4(result, 1.0);
}
