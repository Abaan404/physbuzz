#pbz_include "defines.glsl"

#pbz_include "../shadow/shadow.glsl"

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

vec4 calcPointLight(PointLight light, samplerCube shadowMap, float farPlane, float shininess, vec3 fragPosition, vec3 normal, vec3 viewPosition, vec4 materialDiffuse, vec4 materialSpecular) {
    // diffuse
    vec3 lightDirection = normalize(light.position - fragPosition);
    float diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);

    // inverse square law
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // phong
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;

    // shadows
    float shadow = calcShadowsPoint(shadowMap, fragPosition, viewPosition, normal, lightDirection, light.position, farPlane);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * materialDiffuse.rgb;

    return vec4(lighting, materialDiffuse.a);
}
