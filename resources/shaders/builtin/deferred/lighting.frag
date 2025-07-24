#version 460 core

#pbz_include "../../common/lighting/point.glsl"
#pbz_include "../../common/lighting/directional.glsl"
#pbz_include "../../common/lighting/spot.glsl"

in VS_OUT {
    vec2 texCoord;
} fs_in;

#define MAX_POINT_LIGHTS 100
#define MAX_DIRECTIONAL_LIGHTS 100
#define MAX_SPOT_LIGHTS 100

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

uniform uint PBZ_DirectionalLightLength;
uniform DirectionalLight PBZ_DirectionalLight[MAX_DIRECTIONAL_LIGHTS];

uniform uint PBZ_SpotLightLength;
uniform SpotLight PBZ_SpotLight[MAX_SPOT_LIGHTS];

out vec4 fragColor;

vec4 calcPointLight(PointLight light, vec3 fragPosition, vec3 viewPosition, vec3 normal, vec3 mAlbedo, float mSpecular) {
    // diffuse
    vec3 lightDirection = normalize(light.position - fragPosition);
    float diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(lightDirection + viewPosition);
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);

    // inverse square law
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // phong
    vec3 ambient = light.ambient * mAlbedo;
    vec3 diffuse = light.diffuse * diff * mAlbedo;
    vec3 specular = light.specular * spec * mSpecular;

    return vec4((ambient + diffuse + specular) * attenuation, 1.0f);
}

vec4 calcDirectionalLight(DirectionalLight light, vec3 fragPosition, vec3 viewPosition, vec3 normal, vec3 mAlbedo, float mSpecular) {
    // diffuse
    float diff = max(dot(normal, -light.direction), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(light.direction + viewDirection);
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);

    // phong
    vec3 ambient = light.ambient * mAlbedo;
    vec3 diffuse = light.diffuse * diff * mAlbedo;
    vec3 specular = light.specular * spec * mSpecular;

    return vec4(ambient + diffuse + specular, 1.0f);
}

vec4 calcSpotLight(SpotLight light, vec3 fragPosition, vec3 viewPosition, vec3 normal, vec3 mAlbedo, float mSpecular) {
    // diffuse
    vec3 lightDirection = normalize(light.position - fragPosition);
    float diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);

    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // inverse square law
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // phong
    vec3 ambient = light.ambient * mAlbedo;
    vec3 diffuse = light.diffuse * diff * mAlbedo * intensity;
    vec3 specular = light.specular * spec * mSpecular * intensity;

    return vec4((ambient + diffuse + specular) * attenuation, 1.0f);
}

void main() {
    vec3 fragPosition = texture(PBZ_GBuffer0, fs_in.texCoord).rgb;
    vec3 normal = texture(PBZ_GBuffer1, fs_in.texCoord).rgb;
    vec3 albedo = texture(PBZ_GBuffer2, fs_in.texCoord).rgb;
    float specular = texture(PBZ_GBuffer2, fs_in.texCoord).a;

    vec4 result = vec4(0.0f);
    for (uint i = 0; i < PBZ_PointLightLength; ++i) {
        result += calcPointLight(PBZ_PointLight[i], fragPosition, camera.position, normal, albedo, specular);
    }

    for (uint i = 0; i < PBZ_DirectionalLightLength; ++i) {
        result += calcDirectionalLight(PBZ_DirectionalLight[i], fragPosition, camera.position, normal, albedo, specular);
    }

    for (uint i = 0; i < PBZ_SpotLightLength; ++i) {
        result += calcSpotLight(PBZ_SpotLight[i], fragPosition, camera.position, normal, albedo, specular);
    }

    fragColor = result;
}
