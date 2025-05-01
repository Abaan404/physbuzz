#pbz_include "defines.glsl"

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 calcDirectionalLight(Material material, DirectionalLight light, vec3 fragPosition, vec3 normal, vec2 texCoord, vec3 viewPosition) {
    // diffuse
    float diff = max(dot(normal, -light.direction), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(light.direction, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), material.shininess);

    // phong
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

    return ambient + diffuse + specular;
}
