#pbz_include "blinnphong.glsl"

#define MAX_DIRECTIONAL_LIGHTS 100

struct DirectionalLight {
    Light light;

    vec3 direction;
    mat4 matrix;
};

BlinnPhong calcDirectionalLight(DirectionalLight directional, vec3 fragPosition, vec3 viewPosition, vec3 normal, float shininess, sampler2D shadowMap) {
    BlinnPhong phong;
    phong.light = directional.light;
    phong.attenuation = 1.0f;

    // diffuse
    phong.diff = max(dot(normal, -directional.direction), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(directional.direction + viewDirection);
    phong.spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);

    // shadow
    phong.shadow = 0.0f;

    vec4 lightSpaceFragPosition = directional.matrix * vec4(fragPosition, 1.0);
    vec3 projCoords = lightSpaceFragPosition.xyz / lightSpaceFragPosition.w;
    if (projCoords.z > 1.0) {
        return phong;
    }

    projCoords = projCoords * 0.5f + 0.5f;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.05f * (1.0f - dot(normal, -directional.direction)), 0.005f);

    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            phong.shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    phong.shadow /= 9.0f;

    return phong;
}
