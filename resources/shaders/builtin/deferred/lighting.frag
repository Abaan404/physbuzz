#version 460 core

#pbz_include "../common/lighting/point.glsl"
#pbz_include "../common/lighting/directional.glsl"
#pbz_include "../common/lighting/spot.glsl"

in VS_OUT {
    vec2 texCoord;
} fs_in;

const float shininess = 32.0f; // hardcoded shininess

layout(std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform sampler2D PBZ_GBuffer0; // position
uniform sampler2D PBZ_GBuffer1; // normals
uniform sampler2D PBZ_GBuffer2; // albedo+spec

uniform uint PBZ_PointLightLength;
uniform PointLight PBZ_PointLight[MAX_POINT_LIGHTS];
uniform samplerCube PBZ_PointShadow;

uniform uint PBZ_DirectionalLightLength;
uniform DirectionalLight PBZ_DirectionalLight[MAX_DIRECTIONAL_LIGHTS];
uniform sampler2D PBZ_DirectionalShadow;

uniform uint PBZ_SpotLightLength;
uniform SpotLight PBZ_SpotLight[MAX_SPOT_LIGHTS];

uniform float PBZ_ShadowFarPlane;

out vec4 fragColor;

void main() {
    vec3 fragPosition = texture(PBZ_GBuffer0, fs_in.texCoord).rgb;
    vec3 normal = texture(PBZ_GBuffer1, fs_in.texCoord).rgb;
    vec3 albedo = texture(PBZ_GBuffer2, fs_in.texCoord).rgb;
    float specular = texture(PBZ_GBuffer2, fs_in.texCoord).a;

    BlinnPhong phong;
    vec4 result = vec4(vec3(0.0f), 1.0f);

    for (uint i = 0; i < PBZ_PointLightLength; ++i) {
        phong = calcPointLight(PBZ_PointLight[i], fragPosition, camera.position, normal, shininess, PBZ_ShadowFarPlane, PBZ_PointShadow);
        result.rgb += calcBlinnPhong(phong, albedo, specular);
    }

    for (uint i = 0; i < PBZ_DirectionalLightLength; ++i) {
        phong = calcDirectionalLight(PBZ_DirectionalLight[i], fragPosition, camera.position, normal, shininess, PBZ_DirectionalShadow);
        result.rgb += calcBlinnPhong(phong, albedo, specular);
    }

    for (uint i = 0; i < PBZ_SpotLightLength; ++i) {
        phong = calcSpotLight(PBZ_SpotLight[i], fragPosition, camera.position, normal, shininess);
        result.rgb += calcBlinnPhong(phong, albedo, specular);
    }

    fragColor = result;
}
