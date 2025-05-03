#pbz_include "defines.glsl"

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

vec4 calcSpotLight(Material material, SpotLight light, vec3 fragPosition, vec3 normal, vec3 viewPosition, vec4 materialDiffuse, vec4 materialSpecular) {
    // diffuse
    vec3 lightDirection = normalize(light.position - fragPosition);
    float diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), material.shininess);

    float theta = dot(lightDirection, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // inverse square law
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // phong
    vec3 ambient = light.ambient * materialDiffuse.rgb * attenuation;
    vec3 diffuse = light.diffuse * diff * materialDiffuse.rgb * attenuation * intensity;
    vec3 specular = light.specular * spec * materialSpecular.rgb * attenuation * intensity;

    return vec4(ambient + diffuse + specular, materialDiffuse.a);
}
