#pbz_include "defines.glsl"

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

vec4 calcPointLight(Material material, PointLight light, vec3 fragPosition, vec3 normal, vec3 viewPosition, vec4 materialDiffuse, vec4 materialSpecular) {
    // diffuse
    vec3 lightDirection = normalize(light.position - fragPosition);
    float diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), material.shininess);

    // inverse square law
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // phong
    vec3 ambient = light.ambient * materialDiffuse.rgb * attenuation;
    vec3 diffuse = light.diffuse * diff * materialDiffuse.rgb * attenuation;
    vec3 specular = light.specular * spec * materialSpecular.rgb * attenuation;

    return vec4(ambient + diffuse + specular, materialDiffuse.a);
}
