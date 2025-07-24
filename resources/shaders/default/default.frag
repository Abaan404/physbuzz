#version 460 core

#pbz_include "../common/lighting/point.glsl"
#pbz_include "../common/lighting/directional.glsl"
#pbz_include "../common/lighting/spot.glsl"

#define MAX_SAMPLERS 5
#define MAX_POINT_LIGHTS 100
#define MAX_DIRECTIONAL_LIGHTS 100

layout(std140, binding = 0) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform uint u_MaterialDiffuseLength;
uniform sampler2D u_MaterialDiffuse[MAX_SAMPLERS];

uniform uint u_MaterialSpecularLength;
uniform sampler2D u_MaterialSpecular[MAX_SAMPLERS];

uniform uint u_MaterialHeightLength;
uniform sampler2D u_MaterialHeight[MAX_SAMPLERS];

uniform float u_MaterialShininess;

uniform uint u_PointLightLength;
uniform PointLight u_PointLight[MAX_POINT_LIGHTS];

uniform uint u_DirectionalLightLength;
uniform DirectionalLight u_DirectionalLight[MAX_DIRECTIONAL_LIGHTS];

uniform SpotLight u_SpotLight;

uniform sampler2D u_ShadowMapDirectional;
uniform samplerCube u_ShadowMapPoint;
uniform float u_FarPlane;

uniform samplerCube u_Skybox;

in VS_OUT {
    mat3 TBN;
    vec4 fragPositionLightSpace;
    vec3 normal;
    vec3 fragPosition;
    vec2 texCoord;
} fs_in;

layout(location = 0) out vec4 fragColor;

void main() {
    vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // average the diffuse textures
    vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < u_MaterialDiffuseLength; i++) {
        diffuse += texture(u_MaterialDiffuse[i], fs_in.texCoord);
    }
    diffuse /= float(u_MaterialDiffuseLength);

    // average the specular textures
    vec4 specular = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < u_MaterialSpecularLength; i++) {
        specular += texture(u_MaterialSpecular[i], fs_in.texCoord);
    }
    specular /= float(u_MaterialSpecularLength);

    // average the normal textures
    vec3 normal = vec3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < u_MaterialHeightLength; i++) {
        normal += texture(u_MaterialHeight[i], fs_in.texCoord).rgb;
    }
    normal /= float(u_MaterialHeightLength);

    if (u_MaterialHeightLength > 0) {
        normal = normal * 2.0 - 1.0;
        normal = normalize(fs_in.TBN * normal);
    } else {
        normal = fs_in.normal;
    }

    result += calcSpotLight(u_SpotLight, u_MaterialShininess, fs_in.fragPosition, normal, camera.position, diffuse, specular);

    // vec3 I = normalize(fs_in.fragPosition - camera.position);
    // vec3 R = refract(I, -normalize(fs_in.normal), 1.00 / 1.52);
    // result += vec4(texture(u_Skybox, R).rgb, 1.0);

    for (int i = 0; i < u_DirectionalLightLength; i++) {
        result += calcDirectionalLight(u_DirectionalLight[i], u_ShadowMapDirectional, fs_in.fragPositionLightSpace, u_MaterialShininess, fs_in.fragPosition, normal, camera.position, diffuse, specular);
    }

    for (int i = 0; i < u_PointLightLength; i++) {
        result += calcPointLight(u_PointLight[i], u_ShadowMapPoint, u_FarPlane, u_MaterialShininess, fs_in.fragPosition, normal, camera.position, diffuse, specular);
    }

    fragColor = result;
}
