#version 460 core

#pbz_include "../common/lighting/point.glsl"
#pbz_include "../common/lighting/directional.glsl"
#pbz_include "../common/lighting/spot.glsl"

#define MAX_DIFFUSE_SAMPLERS 5
#define MAX_SPECULAR_SAMPLERS 5

uniform Material u_Material;
uniform sampler2D u_MaterialDiffuse[MAX_DIFFUSE_SAMPLERS];
uniform sampler2D u_MaterialSpecular[MAX_SPECULAR_SAMPLERS];

#define MAX_POINT_LIGHTS 100

uniform uint u_PointLightLength;
uniform PointLight u_PointLight[MAX_POINT_LIGHTS];

uniform DirectionalLight u_DirectionalLight;
uniform SpotLight u_SpotLight;

uniform vec3 u_ViewPosition;

in vec3 normal;
in vec2 texCoord;
in vec3 fragPosition;

out vec4 fragColor;

void main() {
    vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // average the diffuse textures
    vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < u_Material.diffuseLength; i++) {
        diffuse += texture(u_MaterialDiffuse[0], texCoord);
    }
    diffuse /= float(u_Material.diffuseLength);

    // average the specular textures
    vec4 specular = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < u_Material.specularLength; i++) {
        specular += texture(u_MaterialSpecular[0], texCoord);
    }
    specular /= float(u_Material.specularLength);

    result += calcDirectionalLight(u_Material, u_DirectionalLight, fragPosition, normal, u_ViewPosition, diffuse, specular);
    result += calcSpotLight(u_Material, u_SpotLight, fragPosition, normal, u_ViewPosition, diffuse, specular);

    for (int i = 0; i < u_PointLightLength; i++) {
        result += calcPointLight(u_Material, u_PointLight[i], fragPosition, normal, u_ViewPosition, diffuse, specular);
    }

    fragColor = result;
}
