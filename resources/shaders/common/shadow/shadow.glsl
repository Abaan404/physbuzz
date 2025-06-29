float calcShadowsDirectional(sampler2D shadowMap, vec4 lightSpaceFragPosition, vec3 normal, vec3 lightDirection) {
    vec3 projCoords = lightSpaceFragPosition.xyz / lightSpaceFragPosition.w;
    if (projCoords.z > 1.0) {
        return 0.0f;
    }

    projCoords = projCoords * 0.5f + 0.5f;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.05f * (1.0f - dot(normal, -lightDirection)), 0.005f);

    float shadow = 0.0f;
    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;

    return shadow;
}

float calcShadowsPoint(samplerCube shadowMap, vec3 fragPosition, vec3 viewPosition, vec3 normal, vec3 lightDirection, vec3 lightPosition, float farPlane) {
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

    vec3 fragToLight = fragPosition - lightPosition;
    float currentDepth = length(fragToLight);
    float viewDistance = length(viewPosition - fragPosition);

    float bias = 0.0005;
    float shadow = 0.0f;
    int samples = 20;
    float diskRadius = 0.05;
    for (int i = 0; i < samples; ++i) {
        float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= farPlane; // undo mapping [0;1]
        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }
    }
    shadow /= float(samples);

    return shadow;
}
