#pbz_include "blinnphong.glsl"

#define MAX_POINT_LIGHTS 100

struct PointLight {
    Light light;

    vec3 position;

    float constant;
    float linear;
    float quadratic;
};

BlinnPhong calcPointLight(PointLight point, vec3 fragPosition, vec3 viewPosition, vec3 normal, float shininess, float shadowFarPlane, samplerCube shadowMap) {
    BlinnPhong phong;
    phong.light = point.light;

    // diffuse
    vec3 lightDirection = normalize(point.position - fragPosition);
    phong.diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(lightDirection + viewPosition);
    phong.spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);

    // inverse square law
    float distance = length(point.position - fragPosition);
    phong.attenuation = 1.0 / (point.constant + point.linear * distance + point.quadratic * (distance * distance));

    // shadow
    phong.shadow = 0.0f;

    vec3 sampleOffsetDirections[20] = {
            vec3(1, 1, 1),
            vec3(1, -1, 1),
            vec3(-1, -1, 1),
            vec3(-1, 1, 1),
            vec3(1, 1, -1),
            vec3(1, -1, -1),
            vec3(-1, -1, -1),
            vec3(-1, 1, -1),
            vec3(1, 1, 0),
            vec3(1, -1, 0),
            vec3(-1, -1, 0),
            vec3(-1, 1, 0),
            vec3(1, 0, 1),
            vec3(-1, 0, 1),
            vec3(1, 0, -1),
            vec3(-1, 0, -1),
            vec3(0, 1, 1),
            vec3(0, -1, 1),
            vec3(0, -1, -1),
            vec3(0, 1, -1)
        };

    vec3 fragToLight = fragPosition - point.position;
    float currentDepth = length(fragToLight);
    float viewDistance = length(viewPosition - fragPosition);

    float bias = 0.0005;
    int samples = 20;
    float diskRadius = 0.05;
    for (int i = 0; i < samples; ++i) {
        float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= shadowFarPlane; // undo mapping [0;1]
        if (currentDepth - bias > closestDepth) {
            phong.shadow += 1.0;
        }
    }
    phong.shadow /= float(samples);

    return phong;
}
