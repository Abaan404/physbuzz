float calcShadows(sampler2D shadowMap, vec4 lightSpaceFragPos, vec3 normal, vec3 lightDirection) {
    vec3 projCoords = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
    if (projCoords.z > 1.0) {
        return 0.0f;
    }

    projCoords = projCoords * 0.5f + 0.5f;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.05f * (1.0f - dot(normal, -lightDirection)), 0.005f);

    float shadow = 0.0f;
    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;

    return shadow;
}
