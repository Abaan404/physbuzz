#version 460 core

#pbz_include "../common/lighting/point.glsl"
#pbz_include "../common/lighting/directional.glsl"
#pbz_include "../common/lighting/spot.glsl"

#define MAX_SAMPLERS 5

in VS_OUT {
    mat3 TBN;
    vec3 normal;
    vec3 fragPosition;
    vec2 texCoord;
} fs_in;

const float shininess = 32.0f; // hardcoded shininess

layout(std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform uint PBZ_PointLightLength;
uniform PointLight PBZ_PointLight[MAX_POINT_LIGHTS];
uniform samplerCube PBZ_PointShadow;

uniform uint PBZ_DirectionalLightLength;
uniform DirectionalLight PBZ_DirectionalLight[MAX_DIRECTIONAL_LIGHTS];
uniform sampler2D PBZ_DirectionalShadow;

uniform uint PBZ_SpotLightLength;
uniform SpotLight PBZ_SpotLight[MAX_SPOT_LIGHTS];

uniform uint PBZ_TextureDiffuseLength;
uniform sampler2D PBZ_TextureDiffuse[MAX_SAMPLERS];

uniform uint PBZ_TextureSpecularLength;
uniform sampler2D PBZ_TextureSpecular[MAX_SAMPLERS];

uniform uint PBZ_TextureHeightLength;
uniform sampler2D PBZ_TextureHeight[MAX_SAMPLERS];

uniform float PBZ_ShadowFarPlane;

layout(location = 0) out vec4 fragColor;

void main() {
    vec3 albedo = vec3(0.0f);

    for (uint i = 0; i < PBZ_TextureDiffuseLength; i++) {
        albedo += texture(PBZ_TextureDiffuse[i], fs_in.texCoord).rgb;
    }

    if (PBZ_TextureDiffuseLength > 0) {
        albedo /= float(PBZ_TextureDiffuseLength);
    }

    float specular = 0.0f;

    for (uint i = 0; i < PBZ_TextureSpecularLength; ++i) {
        specular += texture(PBZ_TextureSpecular[i], fs_in.texCoord).r;
    }

    if (PBZ_TextureSpecularLength > 0) {
        specular /= float(PBZ_TextureSpecularLength);
    }

    vec3 height = vec3(0.0f);

    if (PBZ_TextureHeightLength > 0) {
        for (uint i = 0; i < PBZ_TextureHeightLength; ++i) {
            height += texture(PBZ_TextureHeight[i], fs_in.texCoord).rgb;
        }

        height /= float(PBZ_TextureHeightLength);
        height = height * 2.0 - 1.0;
        height = normalize(fs_in.TBN * height);
    } else {
        height = normalize(fs_in.normal);
    }

    BlinnPhong phong;
    vec4 result = vec4(vec3(0.0f), 1.0f);

    for (uint i = 0; i < PBZ_PointLightLength; ++i) {
        phong = calcPointLight(PBZ_PointLight[i], fs_in.fragPosition, camera.position, height, shininess, PBZ_ShadowFarPlane, PBZ_PointShadow);
        result.rgb += calcBlinnPhong(phong, albedo, specular);
    }

    for (uint i = 0; i < PBZ_DirectionalLightLength; ++i) {
        phong = calcDirectionalLight(PBZ_DirectionalLight[i], fs_in.fragPosition, camera.position, height, shininess, PBZ_DirectionalShadow);
        result.rgb += calcBlinnPhong(phong, albedo, specular);
    }

    for (uint i = 0; i < PBZ_SpotLightLength; ++i) {
        phong = calcSpotLight(PBZ_SpotLight[i], fs_in.fragPosition, camera.position, height, shininess);
        result.rgb += calcBlinnPhong(phong, albedo, specular);
    }

    fragColor = result;
}
