#pbz_include "defines.glsl"

#pbz_include "../shadow/shadow.glsl"

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    mat4 matrix;
};

vec4 calcDirectionalLight(DirectionalLight light, sampler2D shadowMap, vec4 fragPositionLightSpace, float shininess, vec3 fragPosition, vec3 normal, vec3 viewPosition, vec4 materialDiffuse, vec4 materialSpecular) {
    // diffuse
    float diff = max(dot(normal, -light.direction), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(light.direction + viewDirection);
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);

    // phong
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;

    float shadow = calcShadows(shadowMap, fragPositionLightSpace, normal, light.direction);

    // shadows
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * materialDiffuse.rgb;

    return vec4(lighting, materialDiffuse.a);
}
